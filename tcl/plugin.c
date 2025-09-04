/* lang.tcl plugin for r2 - 2023-2025 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <tcl.h>

typedef struct {
	RCore *core;
	Tcl_Interp *interp;
} TclPluginContext;

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
	return true;
}

static bool fini(RLangSession *s) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_DeleteInterp (pluginContext->interp);
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
