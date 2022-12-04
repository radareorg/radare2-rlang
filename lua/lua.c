/* lang.lua plugin for r2 - 2013-2022 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lstate.h>

#define LIBDIR PREFIX"/lib"

#include "lib/inspect.lua.c"
#undef _BUFFER_SIZE
#include "lib/json.lua.c"
#undef _BUFFER_SIZE
#include "lib/r2api.lua.c"

static void stackDump (lua_State *L) {
	int i, top = lua_gettop (L);
	if (top < 1) {
		return;
	}
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type (L, i);
		switch (t) {
		case LUA_TSTRING:  /* strings */
			printf ("`%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			printf (lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  /* numbers */
			printf ("%g", lua_tonumber(L, i));
			break;
		default:  /* other values */
			printf ("%s", lua_typename(L, t));
			break;
		}
		printf("  ");  /* put a separator */
	}
	printf ("\n");  /* end the listing */
}

static bool lua_run(RLangSession *s, const char *code, int len) {
	RCore *core = s->lang->user;
	lua_State *L = s->plugin_data;
	if (len < 1) {
		len = strlen (code);
	}
	lua_settop (L, 0); // reset the lua stack
	luaL_loadbuffer (L, code, len, ""); // \n included
	if (lua_pcall (L, 0, 0, 1) != LUA_OK) {
		R_LOG_ERROR ("syntax: %s in %s\n", lua_tostring (L, -1), "");
	}
	stackDump (L); // show stack dump if anything is still there
	clearerr (stdin);
	return true;
}

static bool report_error(lua_State *L, int status) {
	if (status) {
		const char *msg = lua_tostring (L, -1);
		if (!msg) {
			msg = "(error with no message)";
		}
		R_LOG_ERROR ("lua error: %s", msg);
		lua_pop (L, 1);
		return false;
	}
	return true;
}

static bool r_lua_file(RLangSession *s, const char *file) {
	lua_State *L = s->plugin_data;
	int res = luaL_loadfile (L, file);
	if (res != LUA_OK) {
		report_error (L, res);
		return false;
	}
	res = lua_pcall (L, 0, 0, 0);
	if (res != LUA_OK) {
		report_error (L, res);
		return false;
	}
	return true;
}

static int lua_cmd_str(lua_State *L) {
	RCore *core = L->l_G->ud_warn;
	const char *s = lua_tostring (L, 1);  /* get argument */
	char *str = r_core_cmd_str (core, s);
	lua_pushstring (L, r_str_get (str));  /* push result */
	free (str);
	return 1;  /* number of results */
}

static int lua_cmd(lua_State *L) {
	RCore *core = L->l_G->ud_warn;
	const char *s = lua_tostring (L, 1);  /* get argument */
	lua_pushnumber (L, r_core_cmd (core, s, 0));  /* push result */
	return 1;  /* number of results */
}

static void *init(RLangSession *s) {
	lua_State *L = luaL_newstate ();
	if (!L) {
		return NULL;
	}
	lua_gc (L, LUA_GCSTOP, 0);
	luaL_openlibs (L);
	luaopen_base (L);
	luaopen_string (L);
	//luaopen_io(L); // PANIC!!
	lua_gc (L, LUA_GCRESTART, 0);
	s->plugin_data = L;

	// we save the RCore pointer into this unused luaglobal state field
	L->l_G->ud_warn = s->lang->user;
#if 0
	// this mode is buggy because scripts can modify this global
	// and make r2cmd calls crash or execute arbitrary code
	lua_pushlightuserdata (L, lang->user);
	lua_setglobal (L, "core");
#endif

	lua_register (L, "r2cmd", &lua_cmd_str);
	lua_pushcfunction (L, lua_cmd_str);
	lua_setglobal (L, "r2cmd");

#if 0
	// DEPRECATED: cmd = radare_cmd_str
	lua_register(G.state, "cmd", &lua_cmd);
	lua_pushcfunction(G.state,lua_cmd);
	lua_setglobal(G.state,"cmd");
#endif
#if 0
	luaL_loadbuffer (G.state, (const char *)json_lua, 9639, "json.lua");
	luaL_loadbuffer (G.state, (const char *)inspect_lua, -1, "inspect.lua");
	if (lua_pcall (G.state, 0, 0, 1) != 0) {
		eprintf ("syntax error(lang_lua): %s in %s\n",
				lua_tostring(G.state, -1), "inspect.lua");
	}
	luaL_loadbuffer (G.state, (const char *)r2api_lua, -1, "r2api.lua");
	if (lua_pcall (G.state, 0, 0, 1) != 0) {
		eprintf ("syntax error(lang_lua): %s in %s\n",
				lua_tostring(G.state, -1), "r2api");
	}
#else
	lua_run (s, (const char *)json_lua, -1);
	lua_run (s, (const char *)r2api_lua, -1);
	lua_run (s, (const char *)inspect_lua, -1);
	lua_run (s, "json.parse = json.decode", 0);
	lua_run (s, "JSON = json", 0);
#endif

	// add custom loader for requiring from memory instead
	lua_run (s, "package.path=os.getenv(\"HOME\")..\"/.local/share/radare2/plugins/lua/?.lua;\"..package.path", 0);
// 	lua_run (s, "json = require \"json\"", 0);
	lua_run (s, "function r2cmdj(x)\n" \
		"	return json.decode(r2cmd(x))\n" 
		"end\n", 0);
/*
	// this requires the native bindings to be built, nobody does that in 2019
	lua_run (s, "require \"r_core\"", 0);
	sprintf (a, "c=r_core.RCore_ncast(0x%"PFMT64x")",
		(ut64)(size_t)(void*)core);
	lua_run (s, a, 0);
*/

	//-- load template
	/// DEPRECATED theres no need for this. better embed everything in into this .c
	// r_lua_file (NULL, LIBDIR"/radare2/"R2_VERSION"/r2api.lua");
	// r_lua_file (NULL, LIBDIR"/radare2/"R2_VERSION"/r2api.lua");
	return L;
}

static RLangPlugin r_lang_plugin_lua = {
	.name = "lua",
	.ext = "lua",
	.desc = "LUA 5.4.4 language extension",
	.run = lua_run,
	.init = init,
	.run_file = r_lua_file,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_lua,
};
#endif
