/* lang.mujs plugin for r2 - 2022 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include "mujs/one.c"
#include "jsapi.c"

typedef struct {
	js_State *J;
	RLangSession *s;
	RCore *core;
	// in case we need to store more data
} MujsContext;

static bool mujs_run(RLangSession *s, const char *code, int len) {
	MujsContext *ctx = s->plugin_data;
	js_State *J = ctx->J;
	char *nc = r_str_newf ("try { var res = (%s); if (res != undefined) { console.log(res); } } catch(e) { console.error(e); }", code);
	js_dostring (J, nc);
	free (nc);
	return true;
}

static bool mujs_file(RLangSession *s, const char *file) {
	MujsContext *ctx = s->plugin_data;
	js_dofile (ctx->J, file);
	return true;
}

static void r2cmd(js_State *J) {
	MujsContext *ctx = J->uctx;
	const char *s = js_tostring (J, 1);
	if (s) {
		char *str = r_core_cmd_str (ctx->core, s);
		js_pushstring (J, str);
		free (str);
	} else {
		js_pushstring (J, "");
	}
}

static bool fini(RLangSession *s) {
	MujsContext *ctx = s->plugin_data;
	js_freestate (ctx->J);
	s->plugin_data = NULL;
	return NULL;
}

static void *init(RLangSession *s) {
	js_State *J = js_newstate (NULL, NULL, JS_STRICT);

	js_newcfunction (J, r2cmd, "r2cmd", 1);
	js_setglobal (J, "r2cmd");

	js_dostring(J, r2_js);
	js_dostring(J, require_js);
	js_dostring(J, stacktrace_js);

	js_newcfunction(J, jsB_print, "print", 0);
	js_setglobal(J, "print");
	js_dostring(J, console_js);

	js_newcfunction(J, jsB_read, "read", 1);
	js_setglobal(J, "read");

	js_newcfunction(J, jsB_gc, "gc", 0);
	js_setglobal(J, "gc");

	MujsContext *ctx = R_NEW0 (MujsContext);
	if (ctx) {
		ctx->s = s;
		ctx->J = J;
		ctx->core = s->lang->user;
		J->uctx = ctx;
	}
	return ctx;
}

static RLangPlugin r_lang_plugin_mujs = {
	.name = "mujs",
	.ext = "mujs",
	.desc = "Ghostscripts mujs interpreter (ES5)",
	.run = mujs_run,
	.init = init,
	.fini = fini,
	.run_file = mujs_file,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_mujs,
};
#endif
