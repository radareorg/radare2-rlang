/* lang.guile plugin for r2 - 2022-2023 - pancake */

#include <r_lib.h>
#include <r_core.h>
#include <r_lang.h>
#include <libguile.h>

static R_TH_LOCAL RCore *Gcore = NULL;

static SCM r2cmd_wrapper(SCM x) {
	size_t len = 0;
	char *cmd = scm_to_utf8_stringn (x, &len);
	char *res = r_core_cmd_str (Gcore, cmd);
	free (cmd);
	return scm_from_utf8_stringn (res, strlen (res));
}

static bool runstr(RLangSession *s, const char *code, int len) {
	scm_c_eval_string (code);
	return false;
}

static int runfile(RLangSession *s, const char *file) {
	char *data = r_file_slurp (file, NULL);
	if (data) {
		scm_c_eval_string (data);
		free (data);
	}
	return 0;
}

static bool init(RLangSession *s) {
	if (s == NULL) {
		return true;
	}
	int argc = 0;
	Gcore = s->lang->user;
	char **argv = NULL;
	scm_init_guile ();
	scm_c_define_gsubr ("r2cmd", 1, 0, 0, r2cmd_wrapper);
	return true;;
}

static RLangPlugin r_lang_plugin_guile = {
	.meta = {
		.name = "guile",
		.license = "LGPL",
		.desc = "GUILE",
		.author = "pancake"
	},
	.ext = "scm",
	.init = (void*)init,
	.run = runstr,
	.run_file = (void*)runfile,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_guile,
};
#endif
