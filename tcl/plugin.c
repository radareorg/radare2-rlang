/* lang.tcl plugin for r2 - 2023-2025 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <r_util.h>
#include <tcl.h>

typedef struct {
	char *name;
	Tcl_Interp *interp;
	Tcl_Obj *call; // TCL command to invoke
} TclCoreHack;

typedef struct {
	RCore *core;
	Tcl_Interp *interp;
	RList *core_plugins; // list of TclCoreHack*
} TclPluginContext;

// Global removed: share context via core->lang->session->plugin_data

static void tcl_free(char *blockPtr) {
	free (blockPtr);
}

static int r2cmd_tcl(void *clientData, Tcl_Interp *interp, int argc, const char **argv) {
	RCore *core = (RCore *)clientData;
	if (argc == 2) {
		char *s = r_core_cmd_str (core, argv[1]);
		Tcl_SetResult (interp, s, (Tcl_FreeProc *)tcl_free);
		return TCL_OK;
	}
	return TCL_ERROR;
}

static void tcl_corehack_free(void *p) {
	TclCoreHack *h = (TclCoreHack *)p;
	if (!h) return;
	if (h->call) {
		Tcl_DecrRefCount (h->call);
	}
	free (h->name);
	free (h);
}

static TclCoreHack *tcl_find_hack(TclPluginContext *ctx, const char *name) {
	if (!ctx || !ctx->core_plugins || !name) {
		return NULL;
	}
	RListIter *it;
	TclCoreHack *h;
	r_list_foreach (ctx->core_plugins, it, h) {
		if (h->name && !strcmp (h->name, name)) {
			return h;
		}
	}
	return NULL;
}

static bool tcl_core_init(RCorePluginSession *cps) {
	if (!cps || !cps->plugin || !cps->plugin->meta.name) {
		return false;
	}
	TclPluginContext *ctx = R_UNWRAP4 (cps->core, lang, session, plugin_data);
	if (!ctx) {
		return false;
	}
	TclCoreHack *h = tcl_find_hack (ctx, cps->plugin->meta.name);
	if (!h) {
		return false;
	}
	cps->data = h;
	return true;
}

static bool tcl_core_fini(RCorePluginSession *cps) {
	// nothing to do per-session; the hack is owned by the lang context list
	return true;
}

static bool tcl_core_call(RCorePluginSession *cps, const char *input) {
	if (!cps || !cps->data) {
		return false;
	}
	TclCoreHack *h = (TclCoreHack *)cps->data;
	Tcl_Interp *interp = h->interp;
	if (!interp || !h->call) {
		return false;
	}
	// Build argv: [call input]
	Tcl_Obj *objv[2];
	objv[0] = h->call;
	objv[1] = Tcl_NewStringObj (input? input: "", -1);
	Tcl_IncrRefCount (objv[1]);
	int rc = Tcl_EvalObjv (interp, 2, objv, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount (objv[1]);
	if (rc == TCL_ERROR) {
		const char *error_msg = Tcl_GetStringResult (interp);
		R_LOG_ERROR ("TCL core plugin error: %s", error_msg);
		Tcl_ResetResult (interp);
		return false;
	}
	// Interpret result
	Tcl_Obj *res = Tcl_GetObjResult (interp);
	int b = 0;
	if (Tcl_GetBooleanFromObj (interp, res, &b) == TCL_OK) {
		Tcl_ResetResult (interp);
		return b ? true : false;
	}
	const char *s = Tcl_GetString (res);
	if (s && *s) {
		r_cons_print (NULL, s);
		r_cons_print (NULL, "\n");
		Tcl_ResetResult (interp);
		return true;
	}
	Tcl_ResetResult (interp);
	return false;
}

static void tcl_core_plugin_free(RCorePlugin *ap) {
	if (!ap) {
		return;
	}
	free ((char *)ap->meta.name);
	free ((char *)ap->meta.license);
	free ((char *)ap->meta.desc);
	free (ap);
}

static int r2plugin_tcl(void *clientData, Tcl_Interp *interp, int argc, const char **argv) {
	TclPluginContext *ctx = (TclPluginContext *)clientData;
	if (!ctx || !ctx->core) {
		return TCL_ERROR;
	}
	if (argc != 3) {
		Tcl_SetResult (interp, "Usage: r2plugin core <providerProc>", TCL_STATIC);
		return TCL_ERROR;
	}
	if (strcmp (argv[1], "core")) {
		Tcl_SetResult (interp, "Only 'core' plugins are supported", TCL_STATIC);
		return TCL_ERROR;
	}
	// Call provider proc with no args; it must return a dict
	Tcl_Obj *prov = Tcl_NewStringObj (argv[2], -1);
	Tcl_IncrRefCount (prov);
	int rc = Tcl_EvalObjv (interp, 1, &prov, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount (prov);
	if (rc == TCL_ERROR) {
		const char *error_msg = Tcl_GetStringResult (interp);
		R_LOG_ERROR ("TCL plugin provider error: %s", error_msg);
		Tcl_ResetResult (interp);
		return TCL_ERROR;
	}
	Tcl_Obj *res = Tcl_GetObjResult (interp);
	if (!res) {
		Tcl_SetResult (interp, "provider returned empty result", TCL_STATIC);
		return TCL_ERROR;
	}
	// Parse dict: name, desc?, license?, call
	Tcl_Obj *val = NULL;
	Tcl_Obj *key = NULL;
	// name
	key = Tcl_NewStringObj ("name", -1);
	Tcl_IncrRefCount (key);
	if (Tcl_DictObjGet (interp, res, key, &val) != TCL_OK || !val) {
		Tcl_DecrRefCount (key);
		Tcl_SetResult (interp, "provider dict must contain 'name'", TCL_STATIC);
		return TCL_ERROR;
	}
	const char *name = Tcl_GetString (val);
	Tcl_DecrRefCount (key);
	if (!name || !*name) {
		Tcl_SetResult (interp, "invalid plugin name", TCL_STATIC);
		return TCL_ERROR;
	}
	// call
	key = Tcl_NewStringObj ("call", -1);
	Tcl_IncrRefCount (key);
	if (Tcl_DictObjGet (interp, res, key, &val) != TCL_OK || !val) {
		Tcl_DecrRefCount (key);
		Tcl_SetResult (interp, "provider dict must contain 'call'", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_Obj *callCmd = val; // keep reference
	Tcl_IncrRefCount (callCmd);
	Tcl_DecrRefCount (key);
	// desc (optional)
	const char *desc = NULL;
	key = Tcl_NewStringObj ("desc", -1);
	Tcl_IncrRefCount (key);
	if (Tcl_DictObjGet (interp, res, key, &val) == TCL_OK && val) {
		desc = Tcl_GetString (val);
	}
	Tcl_DecrRefCount (key);
	// license (optional)
	const char *license = NULL;
	key = Tcl_NewStringObj ("license", -1);
	Tcl_IncrRefCount (key);
	if (Tcl_DictObjGet (interp, res, key, &val) == TCL_OK && val) {
		license = Tcl_GetString (val);
	}
	Tcl_DecrRefCount (key);

	// Check duplicate
	if (tcl_find_hack (ctx, name)) {
		Tcl_DecrRefCount (callCmd);
		Tcl_SetResult (interp, "core plugin already registered", TCL_STATIC);
		return TCL_OK; // mimic other langs: return false-ish; but return OK with message
	}

	// Build and register RCorePlugin
	RCorePlugin *ap = R_NEW0 (RCorePlugin);
	ap->meta.name = strdup (name);
	ap->meta.desc = desc? strdup (desc): NULL;
	ap->meta.license = license? strdup (license): NULL;
	ap->init = tcl_core_init;
	ap->fini = tcl_core_fini;
	ap->call = tcl_core_call;

	TclCoreHack *hack = R_NEW0 (TclCoreHack);
	hack->name = strdup (name);
	hack->interp = interp;
	hack->call = callCmd; // already refcounted
	if (!ctx->core_plugins) {
		ctx->core_plugins = r_list_newf (tcl_corehack_free);
	}
	r_list_append (ctx->core_plugins, hack);

	RLibStruct lp = {
		.type = R_LIB_TYPE_CORE,
		.data = ap,
		.free = (void (*)(void *))tcl_core_plugin_free,
		.version = R2_VERSION,
	};
	int ret = r_lib_open_ptr (ctx->core->lib, ap->meta.name, NULL, &lp);
	// Return boolean result
	Tcl_SetObjResult (interp, Tcl_NewBooleanObj (ret == 1));
	return TCL_OK;
}

static bool runstr(RLangSession *s, const char *code, int len) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_Eval (interp, code) == TCL_ERROR) {
		const char *error_msg = Tcl_GetStringResult (interp);
		R_LOG_ERROR ("TCL Error: %s", error_msg);
		Tcl_ResetResult (interp);
		return true;
	}
	Tcl_ResetResult (interp);
	return false;
}

static bool init(RLangSession * R_NULLABLE s) {
	if (s == NULL) {
		return true;
	}
	TclPluginContext *pluginContext = R_NEW0 (TclPluginContext);
	pluginContext->core = s->lang->user;
	s->plugin_data = pluginContext;
	int argc = 0;
	char **argv = NULL;
	pluginContext->interp = Tcl_CreateInterp ();
	Tcl_Init (pluginContext->interp);
	Tcl_CreateCommand (pluginContext->interp, "r2cmd", r2cmd_tcl, pluginContext->core, NULL);
	Tcl_CreateCommand (pluginContext->interp, "r2plugin", r2plugin_tcl, pluginContext, NULL);
	return true;
}

static bool fini(RLangSession *s) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_DeleteInterp (pluginContext->interp);
	if (pluginContext->core_plugins) {
		r_list_free (pluginContext->core_plugins);
		pluginContext->core_plugins = NULL;
	}
	R_FREE (s->plugin_data);
	return true;
}

static bool runfile(RLangSession *s, const char *file) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	char *data = r_file_slurp (file, NULL);
	if (!data) {
		R_LOG_ERROR ("Failed to slurp file: %s", file);
		return false;
	}
	char *line = r_str_newf ("source %s", file);
	if (!line) {
		free (data);
		return false;
	}
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_Eval (interp, line) == TCL_ERROR) {
		const char *error_msg = Tcl_GetStringResult (interp);
		R_LOG_ERROR ("TCL Error: %s", error_msg);
		free (line);
		free (data);
		Tcl_ResetResult (interp);
		return false;
	}
	free (line);
	free (data);
	Tcl_ResetResult (interp);
	return true;
}

static RLangPlugin r_lang_plugin_tcl = {
	.meta = {
		.name = "tcl",
		.license = "MIT",
		.desc = "TCL/TK scripting for radare2",
		.author = "pancake"
	},
	.ext = "tcl",
	.init = (void*)init,
	.fini = (void*)fini,
	.run = runstr,
	.run_file = (void*)runfile,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_tcl,
};
#endif
