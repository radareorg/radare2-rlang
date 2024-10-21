/* radare - LGPL - Copyright 2014-2024 pancake */

#define _XOPEN_SOURCE
#include <r_core.h>
#include "./duk/duktape.c"
#include "./duk/duk_console.c"

typedef struct {
	RCore *Gcore;
	duk_context *Gctx;
	RArchPlugin *arch_plugin;
	bool init;
	void *Guser;
} R2DukContext;

// static R_TH_LOCAL RArchSession *Gs = NULL; // XXX find a way to duktape userdata
static R_TH_LOCAL R2DukContext *Gk = NULL; // XXX find a way to duktape userdata

static char *mystrdup(const char *s) {
	char *p = NULL;
	if (s) {
		int len = strlen (s) + 1;
		p = malloc (len);
		if (p) {
			memcpy (p, s, len);
		}
	}
	return p;
}

static bool lang_duktape_safe_eval(duk_context *ctx, const char *code);
static void register_r2cmd_duktape(RLangSession *s, duk_context *ctx);
static bool lang_duktape_run(RLangSession *s, const char *code, int len);
static bool lang_duktape_file(RLangSession *s, const char *file);

static bool init(RLangSession *s) {
	R2DukContext *k = s->plugin_data;
	if (k) {
		return false;
	}
	k = R_NEW0 (R2DukContext);
	if (!k) {
		return false;
	}
	Gk = k;
	k->Gctx = duk_create_heap_default ();
        duk_console_init (k->Gctx, DUK_CONSOLE_PROXY_WRAPPER /*flags*/);
	register_r2cmd_duktape (s, k->Gctx);
#if  0
	lang_duktape_safe_eval (Gctx,
		"var console = {log:print,error:print}");
#endif
	lang_duktape_safe_eval (k->Gctx, "function dir(x){"
		"console.log(JSON.stringify(x).replace(/,/g,',\\n '));"
		"for(var i in x) {console.log(i);}}");
	s->plugin_data = k; // implicit
	return true;
}
static bool fini(RLangSession *s) {
	R2DukContext *k = s->plugin_data;
	duk_destroy_heap (k->Gctx);
	k->Gctx = NULL;
	return true;
}

static void pushBuffer(R2DukContext *k, const ut8 *buf, int len) {
	duk_context *Gctx = k->Gctx;
	int i;
	duk_push_fixed_buffer (Gctx, len);
	for (i = 0; i < len; i++) {
		duk_push_number (Gctx, buf[i]);
		duk_put_prop_index (Gctx, -2, i);
	}
	// buffer is in stack[-1]
}

// static int duk_assemble(RArch *a, RAnalOp *op, const char *str) {
static bool duk_assemble(RArchSession *s, RAnalOp *op, RArchEncodeMask mask) {
	R2DukContext *k = s->data;
	duk_context *Gctx = k->Gctx;
	const char *str = op->mnemonic;
	int i, res = 0;
	// call myasm function if available
	duk_push_global_stash (k->Gctx);
	duk_dup (Gctx, 0);  /* timer callback */
	duk_get_prop_string (Gctx, -2, "asmfun");
	k->Guser = duk_require_tval (Gctx, -1);
	if (duk_is_callable (Gctx, -1)) {
		duk_push_string (Gctx, str);
		duk_call (Gctx, 1);
		// [ array of bytes ]
		//duk_dup_top (Gctx);
		res = duk_get_length (Gctx, -1);
		op->size = res;
		ut8 *buf = calloc (res, 1);
		if (buf) {
			for (i = 0; i < res; i++) {
				duk_dup_top (Gctx);
				duk_get_prop_index (Gctx, -2, i);
				buf[i] = duk_to_int (Gctx, -1);
			}
			free (op->bytes);
			op->bytes = r_mem_dup (buf, res);
			op->size = res;
			// r_anal_op_set_bytes (op, op->addr, buf, res);
			free (buf);
		}
	}
	if (res < 1) {
		res = -1;
	}
	return res;
}

static bool duk_disasm(RArchSession *s, RAnalOp *op, RArchDecodeMask mask) {
	R2DukContext *k = Gk; // s->data;
	duk_context *Gctx = k->Gctx;
	int res = 0, res2 = 0;
	const char *opstr = NULL;
	ut8 *b = k->Guser;
	duk_push_global_stash (Gctx);
	duk_dup (Gctx, 0);  /* timer callback */
	duk_get_prop_string (Gctx, -2, "disfun");
	b = k->Guser = duk_require_tval (Gctx, -1);
	const ut8 *buf = op->bytes;
	const int len = op->size;
//	pushBuffer (buf, len);
	if (duk_is_callable (Gctx, -1)) {
		int i;
		// duk_push_string (Gctx, "TODO 2");
		pushBuffer (k, buf, len);
		duk_call (Gctx, 1);

		// [ size, str ]
		for (i = 0; i < 3; i++) {
			duk_dup_top (Gctx);
			duk_get_prop_index (Gctx, -1, i);
			if (duk_is_number (Gctx, -1)) {
				if (res) {
					res2 = duk_to_number (Gctx, -1);
				} else {
					res2 = res = duk_to_number (Gctx, -1);
				}
			} else if (duk_is_string (Gctx, -1)) {
				if (!opstr) {
					opstr = duk_to_string (Gctx, -1);
				}
			}
			duk_pop (Gctx);
		}
	} else {
		R_LOG_ERROR ("[:(] Is not a function %02x %02x", b[0],b[1]);
	}

	// fill op struct
	op->size = res;
	if (!opstr) {
		opstr = "invalid";
	}
	r_asm_op_set_asm (op, opstr);
	char *hexstr = malloc (op->size * 2);
	if (hexstr) {
		r_hex_bin2str (buf, op->size, hexstr);
		r_asm_op_set_hex (op, hexstr);
	}
	return res2;
}

static int r2plugin(duk_context *Gctx) {
	RLibStruct *lib_struct;
	R2DukContext *k = Gk; // XXX find a way to pick a global
	bool ret = true;
	// args: type, function
	const char *type = duk_require_string (Gctx, 0);
	if (strcmp (type, "arch")) {
		R_LOG_TODO ("duk.r2plugin only supports 'arch' plugins atm");
		return false;
	}
	// call function of 2nd parameter, or get object
	if (duk_is_function (Gctx, 1)) {
		duk_push_string (Gctx, "TODO"); // TODO: this must be the RAsm object to get bits, offset, ..
		duk_call (Gctx, 1);
		duk_to_object (Gctx, 1);
	}
	if (!duk_is_object (Gctx, 1)) {
		R_LOG_ERROR ("Expected object or function");
		return false;
	}
	duk_to_object (Gctx, 1);
	#define ap k->arch_plugin
	ap = R_NEW0 (RArchPlugin);

#define GETSTR(x,y,or) \
	duk_dup_top (Gctx); \
	duk_get_prop_string (Gctx, 1, y); \
	if (or) { \
		const char *str = duk_to_string (Gctx, -1); \
		x = mystrdup (str? str: or); \
	} else { \
		x = mystrdup (duk_require_string (Gctx, -1)); \
	} \
	duk_pop (Gctx);

#define GETINT(x,y,or) \
	duk_dup_top (Gctx); \
	duk_get_prop_string (Gctx, 1, y); \
	if (or) { \
		x = duk_is_number (Gctx, -1)? \
			duk_to_int (Gctx, -1): or; \
	} else { \
		x = duk_require_int (Gctx, -1); \
	} \
	duk_pop (Gctx);

#define GETFUN(x,y) \
	duk_dup_top (Gctx); \
	duk_get_prop_string (Gctx, 1, y); \
	x = duk_require_tval (Gctx, 1); \
	duk_pop (Gctx);

	// mandatory
	GETSTR (ap->meta.name, "name", NULL);
	GETSTR (ap->arch, "arch", NULL);
	// optional
	GETSTR (ap->meta.license, "license", "unlicensed");
	GETSTR (ap->meta.desc, "description", "JS Disasm Plugin");
	GETINT (ap->bits, "bits", 32); // XXX use pack!
	// mandatory unless we handle asm+disasm
	k->Guser = duk_require_tval (Gctx, -1);
	// ap->user = duk_dup_top (Gctx); // clone object inside user
	// GETFUN (ap->user, "disassemble");
	duk_push_global_stash (Gctx);
	duk_get_prop_string (Gctx, 1, "disassemble");
	duk_put_prop_string (Gctx, -2, "disfun"); // TODO: prefix plugin name somehow
	// hack to bypass the const callback
	memcpy ((void*)&ap->decode, &duk_disasm, sizeof (ap->decode));

	duk_push_global_stash(Gctx);
	duk_get_prop_string (Gctx, 1, "assemble");
	duk_put_prop_string (Gctx, -2, "asmfun"); // TODO: prefix plugin name somehow
	memcpy ((void*)&ap->encode, &duk_assemble, sizeof (ap->encode));
	// ap->encode = duk_assemble;

#if 0
	duk_get_prop_string (Gctx, 1, "disassemble");
	duk_push_string (Gctx, "WINRAR");
	duk_call (Gctx, 1);
#endif
#if 0
	duk_get_prop_string (Gctx, 1, "disassemble");
	void *a = duk_require_tval (Gctx, -1);
	if (duk_is_callable (Gctx, -1)) {
		ut8 *b = a;
		eprintf ("IS FUNCTION %02x %02x \n", b[0], b[1]);
	} else eprintf ("NOT CALLABLE\n");
	ap->user = a;
	eprintf ("---- %p\n", a);
	duk_push_string (Gctx, "FUCK YOU");
	//duk_dup_top(Gctx);
	//duk_call_method (Gctx, 0);
	duk_call (Gctx, 1);
	duk_push_tval (Gctx, ap->user); // push fun
	duk_push_string (Gctx, "WINRAR");
	duk_call (Gctx, 1);
	duk_pop (Gctx);
#endif

	// TODO: add support to assemble from js too
	//ap->assemble = duk_disasm;
	#define lp lib_struct
	lp = R_NEW0 (RLibStruct);
	if (lp) {
		lp->type = R_LIB_TYPE_ASM; // TODO resolve from handler
		lp->data = ap;
		r_lib_open_ptr (k->Gcore->lib, "duktape.js", NULL, lp);
	}
	duk_push_boolean (Gctx, ret);
	return 1;
}

static int r2cmd(duk_context *ctx) {
	R2DukContext *k = Gk;
	int n = duk_get_top (ctx);  /* #args */
	if (n>0) {
		const char *s = duk_to_string (ctx, 0);
		char *ret = r_core_cmd_str (k->Gcore, s);
		duk_push_string (ctx, ret);
		free (ret);
		return 1;
	}
	return 0;
}

#ifndef PREFIX
#define PREFIX "/usr"
#endif
static void register_r2cmd_duktape(RLangSession *s, duk_context *Gctx) {
	// Gcore = s->lang->user;
	duk_push_global_object (Gctx);

	duk_push_c_function (Gctx, r2cmd, DUK_VARARGS);
	duk_put_prop_string (Gctx, -2 /*idx:global*/, "r2cmd");

	duk_push_c_function (Gctx, r2plugin, DUK_VARARGS);
	duk_put_prop_string (Gctx, -2 /*idx:global*/, "r2plugin");

	duk_pop (Gctx);  /* pop global */
//	lang_duktape_file (lang, "/tmp/r2.js"); ///usr/share/radare2/0.9.8.git/www/t/r2.js");
	lang_duktape_file (s, PREFIX"/share/radare2/last/www/t/r2.js");
}

static void print_error(duk_context *Gctx, FILE *f) {
	if (duk_is_object (Gctx, -1) && duk_has_prop_string (Gctx, -1, "stack")) {
		/* FIXME: print error objects specially */
		/* FIXME: pcall the string coercion */
		duk_get_prop_string (Gctx, -1, "stack");
		if (duk_is_string (Gctx, -1)) {
			fprintf (f, "%s\n", duk_get_string (Gctx, -1));
			fflush (f);
			duk_pop_2 (Gctx);
			return;
		}
		duk_pop (Gctx);
	}
	duk_to_string (Gctx, -1);
	fprintf (f, "%s\n", duk_get_string (Gctx, -1));
	fflush (f);
	duk_pop (Gctx);
}

static int wrapped_compile_execute(duk_context *Gctx, void *usr) {
	duk_compile (Gctx, 0);
	duk_push_global_object (Gctx);
	duk_call_method (Gctx, 0);
// return value is stored here	duk_to_string(Gctx, -1);
	duk_pop (Gctx);
	return 0;
}

static bool lang_duktape_safe_eval(duk_context *Gctx, const char *code) {
#if UNSAFE
	duk_eval_string (Gctx, code);
#else
	bool rc;
	duk_push_lstring (Gctx, code, strlen (code));
	duk_push_string (Gctx, "input");
	rc = duk_safe_call (Gctx, wrapped_compile_execute, NULL, 2, 1);
	if (rc != DUK_EXEC_SUCCESS) {
		print_error (Gctx, stderr);
		rc = false;
	} else {
		duk_pop (Gctx);
		rc = true;
	}
	return rc;
#endif
}

static bool lang_duktape_run(RLangSession *s, const char *code, int len) {
	R2DukContext *k = s->plugin_data;
	return lang_duktape_safe_eval (k->Gctx, code);
}

static bool lang_duktape_file(RLangSession *s, const char *file) {
	R2DukContext *k = s->plugin_data;
	int ret = -1;
	char *code = r_file_slurp (file, NULL);
	if (code) {
		duk_push_lstring (k->Gctx, code, strlen (code));
		duk_push_string (k->Gctx, file);
		free (code);
		ret = duk_safe_call (k->Gctx, wrapped_compile_execute, NULL, 2, 1);
		if (ret != DUK_EXEC_SUCCESS) {
			print_error (k->Gctx, stderr);
			R_LOG_ERROR ("duktape");
		} else {
			duk_pop (k->Gctx);
			ret = 1;
		}
	}
	return ret;
}

static RLangPlugin r_lang_plugin_duktape = {
	.meta = {
		.name = "duktape",
		.desc = "JavaScript extension language using DukTape",
		.license = "LGPL",
	},
	.ext = "duk",
	.run = lang_duktape_run,
	.init = init,
	.fini = fini,
	.run_file = lang_duktape_file,
};

#if !CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_duktape,
	.version = R2_VERSION
};
#endif
