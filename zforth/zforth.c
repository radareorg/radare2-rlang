/* radare - LGPL - Copyright 2024 pancake<nopcode.org> */
/* zForth extension for radare2 */

#include "r_lib.h"
#include "r_lang.h"
#include "zforth.h"

// XXX
//#define RUBYAPI  LIBDIR"/ruby1.8/radare.rb"
#define RUBYAPI  "/usr/lib/radare2/"R2_VERSION"/radare.rb"

#include "r_core.h"

static RCore *Gcore = NULL;

zf_result do_eval(const char *src, int line, const char *buf) {
	const char *msg = NULL;
	zf_result rv = zf_eval (buf);

	switch (rv) {
		case ZF_OK: break;
		case ZF_ABORT_INTERNAL_ERROR: msg = "internal error"; break;
		case ZF_ABORT_OUTSIDE_MEM: msg = "outside memory"; break;
		case ZF_ABORT_DSTACK_OVERRUN: msg = "dstack overrun"; break;
		case ZF_ABORT_DSTACK_UNDERRUN: msg = "dstack underrun"; break;
		case ZF_ABORT_RSTACK_OVERRUN: msg = "rstack overrun"; break;
		case ZF_ABORT_RSTACK_UNDERRUN: msg = "rstack underrun"; break;
		case ZF_ABORT_NOT_A_WORD: msg = "not a word"; break;
		case ZF_ABORT_COMPILE_ONLY_WORD: msg = "compile-only word"; break;
		case ZF_ABORT_INVALID_SIZE: msg = "invalid size"; break;
		case ZF_ABORT_DIVISION_BY_ZERO: msg = "division by zero"; break;
		default: msg = "unknown error";
	}

	if (msg) {
		fprintf(stderr, "\033[31m");
		if(src) fprintf(stderr, "%s:%d: ", src, line);
		fprintf(stderr, "%s\033[0m\n", msg);
	}

	return rv;
}
static void save(const char *fname)
{
	size_t len;
	void *p = zf_dump(&len);
	FILE *f = fopen(fname, "wb");
	if(f) {
		fwrite(p, 1, len, f);
		fclose(f);
	}
}

void include(const char *fname) {
	char buf[256];

	FILE *f = fopen(fname, "rb");
	int line = 1;
	if(f) {
		while(fgets(buf, sizeof(buf), f)) {
			do_eval(fname, line++, buf);
		}
		fclose(f);
	} else {
		fprintf(stderr, "error opening file '%s': %s\n", fname, strerror(errno));
	}
}

  // TODO: use rlog
void zf_host_trace(const char *fmt, va_list va) {
	fprintf(stderr, "\033[1;30m");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\033[0m");
}

  // TODO use rnum
zf_cell zf_host_parse_num(const char *buf) {
	zf_cell v;
	int n = 0;
	int r = sscanf(buf, ZF_SCAN_FMT"%n", &v, &n);
	if(r != 1 || buf[n] != '\0') {
		zf_abort(ZF_ABORT_NOT_A_WORD);
	}
	return v;
}

zf_input_state zf_host_sys(zf_syscall_id id, const char *input) {
	switch ((int)id) {
	/* The core system callbacks */
	case ZF_SYSCALL_EMIT:
		putchar ((char)zf_pop());
		fflush (stdout);
		break;
	case ZF_SYSCALL_PRINT:
		printf(ZF_CELL_FMT " ", zf_pop());
		break;

	case ZF_SYSCALL_TELL: {
		zf_cell len = zf_pop();
		zf_cell addr = zf_pop();
		if(addr >= ZF_DICT_SIZE - len) {
			zf_abort(ZF_ABORT_OUTSIDE_MEM);
		}
		void *buf = (uint8_t *)zf_dump(NULL) + (int)addr;
		(void)fwrite(buf, 1, len, stdout);
		fflush(stdout); }
		break;


	/* Application specific callbacks */

	case ZF_SYSCALL_USER + 0:
		printf("\n");
		exit(0);
		break;

	case ZF_SYSCALL_USER + 1:
		zf_push(sin(zf_pop()));
		break;

	case ZF_SYSCALL_USER + 2:
		if(input == NULL) {
			return ZF_INPUT_PASS_WORD;
		}
		include(input);
		break;
	
	case ZF_SYSCALL_USER + 3:
		save("zforth.save");
		break;

	default:
		printf("unhandled syscall %d\n", id);
		break;
	}

	return ZF_INPUT_INTERPRET;
}

#if 0
static VALUE radare_ruby_cmd(VALUE self, VALUE string) {
	const char *retstr;
	Check_Type (string, T_STRING);
	retstr = r_core_cmd_str (core, RSTRING(string)->ptr);
	if (retstr == NULL || retstr[0]=='\0')
		return rb_str_new2 ("");
	return rb_str_new2 (retstr);
}
#endif

static bool run(RLangSession *user, const char *code, int len) {
	// zf_result err = zf_eval (code);
	zf_result err = do_eval ("-", 0, code);
	if (err != ZF_OK) {
		fprintf (stderr, "error %d handled\n", err);
		return false;
	}
	return true;
}

static bool run_file(RLangSession *user, const char *file) {
	if (r_file_exists (file)) {
		char *contents = r_file_slurp (file, NULL);
		zf_eval (contents);
		return true;
	}
	R_LOG_ERROR ("lang_zforth: Cannot open '%s'\n", file);
	return false;
}

static bool init(RLangSession *user) {
	int trace = 0;
	zf_init (trace);
	zf_bootstrap ();
	include("zForth/forth/core.zf");
	return true;
}

static const char *help =
	"zForth plugin usage:\n"
	" 1 2 + .\n";

static RLangPlugin r_lang_plugin_ruby = {
	.meta = {
		.name = "zforth",
		.desc = "zForth language extension",
	},
	.ext = "zf",
	.init = &init,
	.help = &help,
	.run = &run,
	.run_file = &run_file,
	.set_argv = NULL,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.version = R2_VERSION,
	.data = &r_lang_plugin_ruby,
};
#endif
