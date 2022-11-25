/* lang.lua plugin for r2 - 2013-2022 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LIBDIR PREFIX"/lib"

typedef struct r_lua_state_t {
	lua_State *state;
	RCore* core;
} RLuaState;

static R_TH_LOCAL RLuaState G = { NULL, NULL };

static bool lua_run(RLang *lang, const char *code, int len);

#include "lib/inspect.lua.c"
#undef _BUFFER_SIZE
#include "lib/json.lua.c"
#undef _BUFFER_SIZE
#include "lib/r2api.lua.c"

static int r_lang_lua_report(lua_State *L, int status) {
	const char *msg = NULL;
	if (status) {
		msg = lua_tostring (G.state, -1);
		if (!msg) {
			msg = "(error with no message)";
		}
		eprintf ("status=%d, %s\n", status, msg);
		lua_pop (G.state, 1);
	}
	return status;
}

static int r_lua_file(void *user, const char *file) {
	int status = luaL_loadfile (G.state, file);
	if (status) {
		return r_lang_lua_report (G.state, status);
	}
	status = lua_pcall (G.state, 0, 0, 0);
	return status? r_lang_lua_report (G.state, status): 0;
}

static int lua_cmd_str(lua_State *L) {
	const char *s = lua_tostring (G.state, 1);  /* get argument */
	char *str = r_core_cmd_str (G.core, s);
	lua_pushstring (G.state, str);  /* push result */
	free (str);
	return 1;  /* number of results */
}

static int lua_cmd(lua_State *L) {
	const char *s = lua_tostring (G.state, 1);  /* get argument */
	lua_pushnumber (G.state, r_core_cmd (G.core, s, 0));  /* push result */
	return 1;  /* number of results */
}

#define lua_open()  luaL_newstate()

static int init(RLang *lang) {
	char a[128];
 	G.state = (lua_State*)lua_open();
	if (G.state == NULL) {
		return 0;
	}

	lua_gc (G.state, LUA_GCSTOP, 0);
	luaL_openlibs (G.state);
	luaopen_base (G.state);
	luaopen_string (G.state);
	//luaopen_io(L); // PANIC!!
	lua_gc (G.state, LUA_GCRESTART, 0);

	lua_pushlightuserdata (G.state, lang->user);
	lua_setglobal (G.state, "core");

	lua_register (G.state, "r2cmd", &lua_cmd_str);
	lua_pushcfunction (G.state, lua_cmd_str);
	lua_setglobal (G.state,"r2cmd");
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
	lua_run (lang, (const char *)json_lua, -1);
	lua_run (lang, (const char *)r2api_lua, -1);
	lua_run (lang, (const char *)inspect_lua, -1);
#endif

// add custom loader for requiring from memory instead
	lua_run (lang, "package.path=os.getenv(\"HOME\")..\"/.local/share/radare2/plugins/lua/?.lua;\"..package.path", 0);
// 	lua_run (lang, "json = require \"json\"", 0);
	lua_run (lang, "function r2cmdj(x)\n" \
		"	return json.decode(r2cmd(x))\n" 
		"end\n", 0);
/*
	// this requires the native bindings to be built, nobody does that in 2019
	lua_run (lang, "require \"r_core\"", 0);
	sprintf (a, "c=r_core.RCore_ncast(0x%"PFMT64x")",
		(ut64)(size_t)(void*)core);
	lua_run (lang, a, 0);
*/

	//-- load template
	/// DEPRECATED theres no need for this. better embed everything in into this .c
	// r_lua_file (NULL, LIBDIR"/radare2/"R2_VERSION"/r2api.lua");
	// r_lua_file (NULL, LIBDIR"/radare2/"R2_VERSION"/r2api.lua");
	fflush (stdout);
	return true;
}

static bool lua_run(RLang *lang, const char *code, int len) {
	G.core = lang->user; // XXX buggy?
	if (len < 1) {
		len = strlen (code);
	}
	// RStrBuffer *sb = r_strbuf_buff
	luaL_loadbuffer (G.state, code, len, ""); // \n included
	if (lua_pcall (G.state, 0, 0, 1) != 0) {
		R_LOG_ERROR ("syntax: %s in %s\n", lua_tostring (G.state, -1), "");
	}
	clearerr (stdin);
	// lua_close(L); // TODO
	return true;
}

static RLangPlugin r_lang_plugin_lua = {
	.name = "lua",
	.ext = "lua",
	.desc = "LUA 5.4.4 language extension",
	.run = lua_run,
	.init = (void*)init,
	.run_file = (void*)r_lua_file,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_lua,
};
#endif
