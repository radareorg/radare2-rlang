/* lang.tcl plugin for r2 - 2023-2024 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <tcl.h>

static R_TH_LOCAL RCore *Gcore = NULL;
static R_TH_LOCAL Tcl_Interp *interp = NULL;

static void tcl_free(char *blockPtr) {
	free (blockPtr);
}

static int r2cmd_tcl(void *clientData, Tcl_Interp *interp, int argc, const char **argv) {
	RCore *core = (RCore *)clientData;
	if (argc == 2) {
		char *s = r_core_cmd_str (core, argv[1]);
		Tcl_SetResult (interp, s, tcl_free);
		return TCL_OK;
	}
	return TCL_ERROR;
}

static bool runstr(RLangSession *s, const char *code, int len) {
	if (Tcl_Eval (interp, code) == TCL_ERROR) {
		const char *error_msg = Tcl_GetStringResult (interp);
		R_LOG_ERROR ("Failed to eval %s", error_msg);
		return true;
	}
	return false;
}

static bool init(R_NULLABLE RLangSession *s) {
	if (s == NULL) {
		return true;
	}
	Gcore = s->lang->user;
	int argc = 0;
	char **argv = NULL;
	if (interp == NULL) {
		interp = Tcl_CreateInterp ();
		Tcl_Init (interp);
		Tcl_CreateCommand (interp, "r2cmd", r2cmd_tcl, (ClientData) Gcore, NULL);
	}
	return true;
}

static bool fini(RLangSession *s) {
	Tcl_DeleteInterp (interp);
	interp = NULL;
	return true;
}

static bool runfile(RLangSession *s, const char *file) {
	char *data = r_file_slurp (file, NULL);
	if (data) {
		char *line = r_str_newf ("source %s", file);
		if (Tcl_Eval (interp, line) == TCL_ERROR) {
			const char *error_msg = Tcl_GetStringResult (interp);
			R_LOG_ERROR ("Failed to eval: %s", error_msg);
			free (line);
			return false;
		}
		free (line);
		free (data);
		// required to close the TK script properly..
		fini (s);
		init (s);
	}
	return true;
}

static RLangPlugin r_lang_plugin_tcl = {
	.meta = {
		.name = "tcl",
		.license = "LGPL",
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
