/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include "plugin.h"

static int r2_plugin_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	TclPluginContext *ctx = (TclPluginContext *)clientData;
	if (!ctx || !ctx->core) {
		return TCL_ERROR;
	}
	if (objc != 3) {
		Tcl_WrongNumArgs (interp, 1, objv, "type provider");
		return TCL_ERROR;
	}
	const char *type = Tcl_GetString (objv[1]);
	Tcl_Obj *spec = tcl_resolve_provider (interp, objv[2]);
	if (!spec) {
		return TCL_ERROR;
	}
	int ret = 0;
	if (!strcmp (type, "core")) {
		ret = tcl_register_core_plugin (ctx, interp, spec);
	} else if (!strcmp (type, "io")) {
		ret = tcl_register_io_plugin (ctx, interp, spec);
	} else if (!strcmp (type, "arch")) {
		ret = tcl_register_arch_plugin (ctx, interp, spec);
	} else {
		tcl_obj_unref (spec);
		Tcl_SetResult (interp, "unsupported plugin type", TCL_STATIC);
		return TCL_ERROR;
	}
	tcl_obj_unref (spec);
	Tcl_SetObjResult (interp, Tcl_NewBooleanObj (ret == 1));
	return TCL_OK;
}

static bool runstr(RLangSession *s, const char *code, int len) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_Eval (interp, code) == TCL_ERROR) {
		R_LOG_ERROR ("TCL Error: %s", Tcl_GetStringResult (interp));
		tcl_reset_result (interp);
		return false;
	}
	tcl_reset_result (interp);
	return true;
}

static bool init(RLangSession *R_NULLABLE s) {
	if (s == NULL) {
		return true;
	}
	TclPluginContext *pluginContext = R_NEW0 (TclPluginContext);
	if (!pluginContext) {
		return false;
	}
	pluginContext->core = s->lang->user;
	pluginContext->interp = Tcl_CreateInterp ();
	if (!pluginContext->interp) {
		free (pluginContext);
		return false;
	}
	if (Tcl_Init (pluginContext->interp) != TCL_OK) {
		R_LOG_ERROR ("TCL init error: %s", Tcl_GetStringResult (pluginContext->interp));
		Tcl_DeleteInterp (pluginContext->interp);
		free (pluginContext);
		return false;
	}
	tcl_install_constants (pluginContext->interp);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::cmd", r2_cmd_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::print", r2_print_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::flush", r2_flush_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::const", r2_const_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::plugin", r2_plugin_objcmd, pluginContext, NULL);
	Tcl_Eval (pluginContext->interp,
		"namespace eval ::r2 { namespace export cmd print flush const plugin; namespace ensemble create }\n"
		"interp alias {} r2cmd {} ::r2::cmd\n"
		"interp alias {} r2plugin {} ::r2::plugin\n"
		"interp alias {} r2print {} ::r2::print\n"
		"interp alias {} r2flush {} ::r2::flush\n"
		"interp alias {} r2const {} ::r2::const\n");
	s->plugin_data = pluginContext;
	return true;
}

static bool fini(RLangSession *s) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	if (!pluginContext) {
		return true;
	}
	r_list_free (pluginContext->core_plugins);
	r_list_free (pluginContext->io_plugins);
	r_list_free (pluginContext->arch_plugins);
	if (pluginContext->interp) {
		Tcl_DeleteInterp (pluginContext->interp);
	}
	R_FREE (s->plugin_data);
	return true;
}

static bool runfile(RLangSession *s, const char *file) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_EvalFile (interp, file) == TCL_ERROR) {
		R_LOG_ERROR ("TCL Error: %s", Tcl_GetStringResult (interp));
		tcl_reset_result (interp);
		return false;
	}
	tcl_reset_result (interp);
	return true;
}

static RLangPlugin r_lang_plugin_tcl = {
	.meta = {
		.name = "tcl",
		.license = "MIT",
		.desc = "TCL/TK scripting for radare2",
		.author = "pancake" },
	.ext = "tcl",
	.init = (void *)init,
	.fini = (void *)fini,
	.run = runstr,
	.run_file = (void *)runfile,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_tcl,
};
#endif
