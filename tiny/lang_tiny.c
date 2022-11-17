#include "tiny.h"
#include <r_core.h>

// TODO move into Tiny_StateThread
static RCore *Gcore = NULL;

const Tiny_NativeProp r2prop = {
	"r2core",
	NULL,
	NULL,
};

static Tiny_Value Lib_r2trim(Tiny_StateThread *thread, const Tiny_Value *args, int count) {
	// thread->userdata = NULL;
	const char *cmd = Tiny_ToString (args[0]);
	char *res = strdup (cmd);
	r_str_trim (res);
	return Tiny_NewString(thread, res);
}

static Tiny_Value Lib_r2atoi(Tiny_StateThread *thread, const Tiny_Value *args, int count) {
	const char *cmd = Tiny_ToString (args[0]);
	ut64 n = r_num_math (NULL, cmd);
	return Tiny_NewInt(n);
}

static Tiny_Value Lib_r2cmd(Tiny_StateThread *thread, const Tiny_Value *args, int count) {
	const char *cmd = Tiny_ToString (args[0]);
#if 0
	Tiny_Value v = Tiny_GetGlobal(thread, 0);
	void *core = Tiny_ToAddr (v);
#else
	void *core = thread->userdata; // Gcore;
#endif
	if (!core) {
		printf ("No core pointer\n");
		return Tiny_Null;
	}
	char *res = r_core_cmd_str (core, cmd);
	if (!res || !*res) {
		free (res);
		return Tiny_Null;
	}
	return Tiny_NewString(thread, res);
}

#if 0
int main(int argc, char **argv) {
	if (argc > 1) {
		tinyrun (NULL, argv[1], NULL);
		return 0;
	}
	return 1;
}
#endif

static int tinyrun(RCore *core, const char *filename, const char *eval) {
	Tiny_State *ts = Tiny_CreateState();

	// Tiny_BindStandardRadare2 (ts);
	Tiny_BindStandardArray (ts);
	Tiny_BindStandardDict (ts);
	Tiny_BindStandardIO (ts);
	Tiny_BindStandardLib (ts);
	Tiny_BindFunction(ts, "r2(str): str", Lib_r2cmd);
	Tiny_BindFunction(ts, "trim(str): str", Lib_r2trim);
	Tiny_BindFunction(ts, "atoi(str): int", Lib_r2atoi);
	if (filename) {
		Tiny_CompileFile (ts, filename);
	} else if (eval) {
		Tiny_CompileString (ts, "main", eval);
	}
	Gcore = core;
#if 0
	RCore *core = r_core_new ();
	char *s = r_str_newf ("r2core := %lld", (unsigned long long)(size_t)core);
	Tiny_CompileString (ts, "r2core", s);
#endif
	Tiny_StateThread thread;
	Tiny_InitThread(&thread, ts);
	Tiny_StartThread(&thread);
	thread.userdata = core;

#if 0
	Tiny_Value v = Tiny_NewNative (&thread, core, &r2prop);
	Tiny_Value tiny_core = Tiny_NewLightNative (core);
	Tiny_CallFunction (&thread, 0, NULL, 0);
	// Tiny_SetGlobal (&thread, 0, v);
#endif

	while (Tiny_ExecuteCycle(&thread))
		;
	// GenerateCode (ts, TINY_OP_HALT);
	// CheckInitialized(ts);  // Done after compilation because it might have registered
	// Tiny_ExecuteCycle
	// CompileFile()
	return 0;
}

static bool lang_tiny_run(RLang *lang, const char *code, int len) {
	RCore *core = lang->user;
	return tinyrun (core, NULL, code);
}

static int lang_tiny_file(RLang *lang, const char *file) {
	RCore *core = lang->user;
	return tinyrun (core, file, NULL);
}

static RLangPlugin r_lang_plugin_tiny = {
	.name = "tiny",
	.ext = "tiny",
	.desc = "Tiny programming language",
	.license = "MIT",
	.run = lang_tiny_run,
	.run_file = (void*)lang_tiny_file,
};

#if !CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_tiny,
	.version = R2_VERSION
};
#endif
