/* radare - LGPL - Copyright 2020-2022 pancake */

#define _XOPEN_SOURCE 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <r_lib.h>
#include <r_core.h>
#include <r_config.h>
#include <r_lang.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#if QJS_FRIDA
#include "quickjs-frida/quickjs.h"
#else
#include "quickjs-bellard/quickjs.h"
#endif

// TODO: remove all those globals
static R_TH_LOCAL JSContext *ctx = NULL;
static R_TH_LOCAL JSRuntime *rt = NULL;
static R_TH_LOCAL RCore *Gcore = NULL;
static R_TH_LOCAL bool is_init = false;

static void js_dump_obj(JSContext *ctx, FILE *f, JSValueConst val) {
	const char *str = JS_ToCString (ctx, val);
	if (str) {
		fprintf (f, "%s\n", str);
		JS_FreeCString (ctx, str);
	} else {
		fprintf (f, "[exception]\n");
	}
}

////////////////////////////////
#if QJS_LIBC

#if QJS_FRIDA
#include "quickjs-frida/quickjs-libc.h"
#else
#include "quickjs-bellard/quickjs-libc.h"
#endif

#else

#include "quickjs-frida/quickjs.h"


static void js_std_dump_error1(JSContext *ctx, JSValueConst exception_val) {
	JSValue val;
	bool is_error;

	is_error = JS_IsError(ctx, exception_val);
	js_dump_obj(ctx, stderr, exception_val);
	if (is_error) {
		val = JS_GetPropertyStr (ctx, exception_val, "stack");
		if (!JS_IsUndefined (val)) {
			js_dump_obj (ctx, stderr, val);
		}
		JS_FreeValue (ctx, val);
	}
}

void js_std_dump_error(JSContext *ctx) {
	JSValue exception_val;
	exception_val = JS_GetException (ctx);
	js_std_dump_error1 (ctx, exception_val);
	JS_FreeValue (ctx, exception_val);
}

#endif
////////////////////////////////

static JSValue r2log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	r_cons_printf ("%s\n", n);
	return JS_NewBool (ctx, true);
}

static JSValue r2error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2 (ctx, &plen, argv[0], false);
	eprintf ("%s\n", n);
	return JS_NewBool (ctx, true);
}

static JSValue r2cmd(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	size_t plen;
	const char *n = JS_ToCStringLen2(ctx, &plen, argv[0], false);
	char *ret = r_core_cmd_str (Gcore, n);
	return JS_NewString (ctx, ret);
}

static const JSCFunctionListEntry js_r2_funcs[] = {
	JS_CFUNC_DEF ("cmd", 1, r2cmd),
	JS_CFUNC_DEF ("log", 1, r2log),
	JS_CFUNC_DEF ("error", 1, r2error),
};

static int js_r2_init(JSContext *ctx, JSModuleDef *m) {
	return JS_SetModuleExportList (ctx, m, js_r2_funcs, countof (js_r2_funcs));
}

static JSContext *JS_NewCustomContext(JSRuntime *rt) {
	JSContext *ctx = JS_NewContext (rt);
	// JSContext *ctx = JS_NewContextRaw (rt);
	if (!ctx) {
		return NULL;
	}
#ifdef CONFIG_BIGNUM
	if (bignum_ext) {
		JS_AddIntrinsicBigFloat(ctx);
		JS_AddIntrinsicBigDecimal(ctx);
		JS_AddIntrinsicOperators(ctx);
		JS_EnableBignumExt(ctx, TRUE);
	}
#endif
#if QJS_LIBC
	js_std_init_handlers(rt);
	/* system modules */
	js_init_module_os(ctx, "os");
	js_init_module_std(ctx, "std");
#if 0
	// if load_std
	const char *str = "import * as std from 'std';\n"
		"import * as os from 'os';\n"
		"globalThis.std = std;\n"
		"globalThis.os = os;\n";
	eval_buf(ctx, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
#endif
#endif
	return ctx;
}

static void register_helpers(RLang *lang) {
	// TODO: move this code to init/fini
	if (ctx || is_init) {
	//	return;
	}
	is_init = true;
	Gcore = lang->user;
	rt = JS_NewRuntime ();
#if QJS_LIBC
	js_std_set_worker_new_context_func (JS_NewCustomContext);
	js_std_init_handlers (rt);
	ctx = JS_NewCustomContext (rt);
	JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);
#else
	ctx = JS_NewCustomContext (rt);
#endif
	JSModuleDef *m = JS_NewCModule (ctx, "r2", js_r2_init);
	if (!m) {
		return ;
	}
	js_r2_init (ctx, m);
	JS_AddModuleExportList (ctx, m, js_r2_funcs, countof (js_r2_funcs));
#if 0
	eval(ctx, "function dir(x){"
			"console.log(JSON.stringify(x).replace(/,/g,',\\n '));"
			"for(var i in x) {console.log(i);}}");
#endif
}

static void eval_jobs(JSContext *ctx) {
	JSContext * pctx = NULL;
	do {
		int res = JS_ExecutePendingJob (rt, &pctx);
		if (res == -1) {
			eprintf ("exception in job%c", 10);
		}
	} while (pctx);
}

static bool eval(JSContext *ctx, const char *code) {
	JSValue v = JS_Eval (ctx, code, strlen (code), "-", 0);
	if (JS_IsException (v)) {
		js_std_dump_error (ctx);
		JSValue e = JS_GetException (ctx);
		js_dump_obj (ctx, stderr, e);
	}
	eval_jobs (ctx);
	JS_FreeValue (ctx, v);
	return true;
}

static bool lang_quickjs_run(RLangSession *s, const char *code, int len) {
	register_helpers (s->lang);
	return eval (ctx, code);
}

static int lang_quickjs_file(RLang *lang, const char *file) {
	int rc = -1;
	register_helpers (lang);
	char *code = r_file_slurp (file, NULL); 
	if (code) {
		rc = eval (ctx, code);
		free (code);
	}
	return rc;
}

static RLangPlugin r_lang_plugin_quickjs = {
	.name = "qjs",
	.ext = "qjs",
	.license = "MIT",
	.desc = "JavaScript extension language using QuicKJS",
	.run = lang_quickjs_run,
	.run_file = (void*)lang_quickjs_file,
};

#if !CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_quickjs,
	.version = R2_VERSION
};
#endif
