/* radare - LGPL - Copyright 2019-2025 pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <r_list.h>

static const char *r2v_sym = "r2v__entry";

typedef bool (*VCoreCall)(RCore *core, const char *input);

typedef struct {
	char *name;
	void *lib; // dl handle to keep V shared object alive
	VCoreCall call;
} VCoreHack;

typedef struct {
	RCore *core;
	RList *libs;         // list of void* dl handles
	RList *core_plugins; // list of VCoreHack*
} VPluginContext;

static void v_corehack_free(void *p) {
	VCoreHack *h = (VCoreHack *)p;
	if (!h) return;
	free (h->name);
	// do not close h->lib here; libs list owns the handle
	free (h);
}

static VCoreHack *v_find_hack(VPluginContext *ctx, const char *name) {
	if (!ctx || !ctx->core_plugins || !name) return NULL;
	RListIter *it; VCoreHack *h;
	r_list_foreach (ctx->core_plugins, it, h) {
		if (h->name && !strcmp (h->name, name)) return h;
	}
	return NULL;
}

// Thread-local/current session pointer for callbacks from V
static VPluginContext *current_ctx = NULL;

static bool lang_v_file(RLangSession *s, const char *file);

static const char *r2v_head = \
			      "module r2v\n"
			      "\n";

			      static const char *r2v_body = \
							    "#pkgconfig --cflags --libs r_core\n"
							    "\n"
							    "#include <r_core.h>\n"
							    "\n"
							    "struct R2 {}\n"
							    "fn C.r_core_cmd_str (core &R2, s byteptr) byteptr\n"
							    "fn C.r_core_free (core &R2)\n"
							    "fn C.r_core_new () &R2\n"
							    "\n"
							    "// Core plugin registration bridge\n"
							    "type CoreCallFn = fn(&R2, byteptr) bool\n"
							    "fn C.r2v_register_core(name byteptr, call CoreCallFn, desc byteptr, license byteptr) bool\n"
							    "\n"
							    "pub fn (core &R2)register_core(name string, call CoreCallFn, desc string, license string) bool {\n"
							    "  return C.r2v_register_core(name.str, call, desc.str, license.str)\n"
							    "}\n"
							    "\n"
							    "pub fn (core &R2)cmd(s string) string {\n"
							    "  unsafe {\n"
							    "    o := C.r_core_cmd_str (core, s.str)\n"
							    "    strs := o.vstring()\n"
							    "    // free(o)\n"
							    "    return strs\n"
							    "  }\n"
							    "}\n"
							    "\n"
							    "pub fn (core &R2)str() string {\n"
							    "        return i64(core).str()\n"
							    "}\n"
							    "\n"
							    "pub fn (core &R2)free() {\n"
							    "        unsafe {C.r_core_free (core)}\n"
							    "}\n"
							    "\n"
							    "fn new() &R2 {\n"
							    "        return C.r_core_new ()\n"
							    "}\n";

							    typedef struct VParse {
								    RStrBuf *head;
								    RStrBuf *body;
							    } VParse;

static void vcode_fini(VParse *p) {
	r_strbuf_free (p->head);
	r_strbuf_free (p->body);
}

static VParse vcode_parse(const char *code) {
	VParse vp = (VParse){0};
	vp.head = r_strbuf_new ("");
	vp.body = r_strbuf_new ("");
	if (!code) {
		return vp;
	}
	char *c = strdup (code);
	if (!c) {
		return vp;
	}
	char *p = c;
	for (char *cp = c; ; cp++) {
		if (*cp == '\n' || !*cp) {
			char save = *cp;
			*cp = 0;
			if (*p) {
				if (r_str_startswith (p, "module ")) {
					// skip explicit module lines
				} else if (r_str_startswith (p, "import ")) {
					r_strbuf_appendf (vp.head, "%s\n", p);
				} else {
					r_strbuf_appendf (vp.body, "%s\n", p);
				}
			}
			if (!save) {
				break;
			}
			*cp = save;
			p = cp + 1;
		}
	}
	free (c);
	return vp;
}

static void runlib(void *user, const char *lib) {
	void *vl = r_lib_dl_open (lib, false);
	if (vl) {
		void (*fcn)(RCore *, int argc, const char **argv);
		fcn = r_lib_dl_sym (vl, r2v_sym);
		if (fcn) {
			fcn (user, 0, NULL);
		} else {
			eprintf ("Cannot find '%s' symbol in library\n", r2v_sym);
		}
		r_lib_dl_close (vl);
	} else {
		eprintf ("Cannot open '%s' library\n", lib);
	}
}

static bool __run(RLangSession *s, const char *code, int len) {
	r_file_rm (".tmp.v");
	FILE *fd = r_sandbox_fopen (".tmp.v", "w");
	if (!fd) {
		eprintf ("Cannot open .tmp.v\n");
		return false;
	}
	VParse vcode = vcode_parse (code);
	fputs (r2v_head, fd);
	fputs (r_strbuf_get (vcode.head), fd);
	fputs (r2v_body, fd);
	const char *body = r_strbuf_get (vcode.body);
	bool has_entry = body && strstr (body, "fn entry") != NULL;
	if (!has_entry) {
		fputs ("pub fn entry(r2 &R2) {\n", fd);
	}
	fputs (body ? body : "", fd);
	if (!has_entry) {
		fputs ("}\n", fd);
	}
	fclose (fd);
	bool ok = lang_v_file (s, ".tmp.v");
	vcode_fini (&vcode);
	return ok;
}

static bool lang_v_file(RLangSession *s, const char *file) {
	if (!s || !s->lang || R_STR_ISEMPTY (file)) {
		return false;
	}
	if (!r_str_endswith (file, ".v")) {
		return false;
	}
	if (strcmp (file, ".tmp.v")) {
		char *code = r_file_slurp (file, NULL);
		bool r = __run (s, code, -1);
		free (code);
		return r;
	}
	if (!r_file_exists (file)) {
		eprintf ("file not found (%s)\n", file);
		return false;
	}
	char *name = strdup (file);
	char *a = (char*)r_str_lchr (name, '/');
	const char *libpath, *libname;
	if (a) {
		*a = 0;
		libpath = name;
		libname = a + 1;
	} else {
		libpath = ".";
		libname = name;
	}
	r_sys_setenv ("PKG_CONFIG_PATH", R2_LIBDIR"/pkgconfig");
	char *lib = r_str_replace (strdup (file), ".v", "."R_LIB_EXT, 1);
	char *cmd = r_str_newf ("v -shared %s", file);
	bool ok = false;
	if (cmd) {
		if (r_sandbox_system (cmd, 1) == 0) {
			// Keep the library open for the session lifetime
			void *vl = r_lib_dl_open (lib, false);
			if (vl) {
				// store handle in session context
				VPluginContext *ctx = (VPluginContext *)s->plugin_data;
				if (!ctx) {
					// create minimal context if init() wasn't called
					ctx = R_NEW0 (VPluginContext);
					ctx->core = s->lang->user;
					s->plugin_data = ctx;
				}
				if (!ctx->libs) ctx->libs = r_list_newf (NULL);
				r_list_append (ctx->libs, vl);

				void (*fcn)(RCore *, int argc, const char **argv) = NULL;
				fcn = r_lib_dl_sym (vl, r2v_sym);
				if (fcn) {
					current_ctx = ctx; // enable registration callbacks
					fcn (s->lang->user, 0, NULL);
					current_ctx = NULL;
					ok = true;
				} else {
					eprintf ("Cannot find '%s' symbol in library\n", r2v_sym);
				}
			} else {
				eprintf ("Cannot open '%s' library\n", lib);
			}
		}
	}
	free (cmd);
	if (lib) {
		r_file_rm (lib);
		free (lib);
	}
	free (name);
	return ok;
}

static bool lang_v_run(RLangSession *s, const char *code, int len) {
	return __run (s, code, len);
}

// Core plugin session bridge
static bool v_core_init(RCorePluginSession *cps) {
	if (!cps || !cps->plugin || !cps->plugin->meta.name) return false;
	VPluginContext *ctx = R_UNWRAP4 (cps->core, lang, session, plugin_data);
	if (!ctx) return false;
	VCoreHack *h = v_find_hack (ctx, cps->plugin->meta.name);
	if (!h) return false;
	cps->data = h;
	return true;
}

static bool v_core_fini(RCorePluginSession *cps) {
	return true;
}

static bool v_core_call(RCorePluginSession *cps, const char *input) {
	if (!cps || !cps->data) return false;
	VCoreHack *h = (VCoreHack *)cps->data;
	if (!h->call) return false;
	return h->call (cps->core, input ? input : "");
}

static void v_core_plugin_free(RCorePlugin *ap) {
	if (!ap) return;
	free ((char *)ap->meta.name);
	free ((char *)ap->meta.license);
	free ((char *)ap->meta.desc);
	free (ap);
}

// Exposed to V code: register a core plugin
R_API bool r2v_register_core(const char *name, VCoreCall call, const char *desc, const char *license) {
	if (!current_ctx || !current_ctx->core || !name || !*name || !call) {
		return false;
	}
	if (!current_ctx->core_plugins) {
		current_ctx->core_plugins = r_list_newf (v_corehack_free);
	}
	if (v_find_hack (current_ctx, name)) {
		return false; // already exists
	}

	RCorePlugin *ap = R_NEW0 (RCorePlugin);
	ap->meta.name = strdup (name);
	ap->meta.desc = desc ? strdup (desc) : NULL;
	ap->meta.license = license ? strdup (license) : NULL;
	ap->init = v_core_init;
	ap->fini = v_core_fini;
	ap->call = v_core_call;

	VCoreHack *hack = R_NEW0 (VCoreHack);
	hack->name = strdup (name);
	hack->call = call;
	// keep last opened lib alive by associating with registration context
	// The lib handle is already stored in ctx->libs when entry runs
	if (current_ctx->libs && current_ctx->libs->tail) {
		hack->lib = current_ctx->libs->tail->data;
	}
	r_list_append (current_ctx->core_plugins, hack);

	RLibStruct lp = {
		.type = R_LIB_TYPE_CORE,
		.data = ap,
		.free = (void (*)(void *))v_core_plugin_free,
		.version = R2_VERSION,
	};
	int ret = r_lib_open_ptr (current_ctx->core->lib, ap->meta.name, NULL, &lp);
	return ret == 1;
}

static bool v_init(RLangSession *s) {
	if (!s) return true;
	VPluginContext *ctx = R_NEW0 (VPluginContext);
	ctx->core = s->lang->user;
	s->plugin_data = ctx;
	return true;
}

static bool v_fini(RLangSession *s) {
	VPluginContext *ctx = (VPluginContext *)s->plugin_data;
	if (ctx) {
		if (ctx->core_plugins) {
			r_list_free (ctx->core_plugins);
			ctx->core_plugins = NULL;
		}
		if (ctx->libs) {
			// close all kept dl handles
			RListIter *it; void *h;
			r_list_foreach (ctx->libs, it, h) {
				if (h) r_lib_dl_close (h);
			}
			r_list_free (ctx->libs);
			ctx->libs = NULL;
		}
		R_FREE (s->plugin_data);
	}
	return true;
}

static RLangPlugin r_lang_plugin_v = {
	.meta = {
		.name = "v",
		.author = "pancak",
		.license = "MIT",
		.desc = "V language extension",
	},
	.ext = "v",
	.init = (void*)v_init,
	.fini = (void*)v_fini,
	.run = lang_v_run,
	.run_file = (void*)lang_v_file,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_v,
};
#endif
