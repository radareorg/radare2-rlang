/* tcc extension for libr (radare2) - copyright pancake 2011-2024 */

#include <r_lib.h>
#include <r_lang.h>
#include <libtcc.h>

/* TODO: store the state globally or so.. */
static bool r_lang_tcc_run(RLangSession *ls, const char *code, int len) {
	TCCState *ts = tcc_new ();
	/* TODO: set defined vars as global */
	// list_for_each(lang->defs) {
	tcc_compile_string (ts, code);
	tcc_run (ts, 0, 0); //argc, argv);
	tcc_delete (ts);
	return R_TRUE;
}

static RLangPlugin r_lang_plugin_tcc = {
	.meta = {
		.name = "tcc",
		.desc = "C99 rlang extension using libtcc",
	},
	.ext = "c",
	.run = &r_lang_tcc_run
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.version = R2_VERSION,
	.data = &r_lang_plugin_tcc,
};
#endif
