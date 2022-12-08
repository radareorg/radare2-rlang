/* lang.mujs plugin for r2 - 2022 - pancake */

static const char *r2_js =
	"var r2 = {cmd: r2cmd, cmdj: function(x) {\n"
	"  return JSON.parse(r2cmd(x));\n"
	" }\n"
	"};\n"
;
static const char *require_js =
	"function inspect(x) { try { var o = JSON.parse(x); } catch (e) { o = x; }\nreturn JSON.stringify(o,null, 2); }\n"
	"var exports = {}\n"
	"function require(name) {\n"
	"var cache = require.cache;\n"
	"if (name in cache) return cache[name];\n"
	"var exports = {};\n"
	"cache[name] = exports;\n"
	"Function('exports', read(name+'.js'))(exports);\n"
	"return exports;\n"
	"}\n"
	"require.cache = Object.create(null);\n"
;


static const char *stacktrace_js =
	"Error.prototype.toString = function() {\n"
	"var s = this.name;\n"
	"if ('message' in this) s += ': ' + this.message;\n"
	"if ('stackTrace' in this) s += this.stackTrace;\n"
	"return s;\n"
	"};\n"
;
static const char *console_js =
	"var console = { log: print, debug: print, warn: print, error: print };"
;

static void jsB_read(js_State *J) {
	// TODO: check r2 sandbox
	const char *filename = js_tostring(J, 1);
	FILE *f;
	char *s;
	int n, t;

	f = fopen(filename, "rb");
	if (!f) {
		js_error(J, "cannot open file '%s': %s", filename, strerror(errno));
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		fclose(f);
		js_error(J, "cannot seek in file '%s': %s", filename, strerror(errno));
	}

	n = ftell(f);
	if (n < 0) {
		fclose(f);
		js_error(J, "cannot tell in file '%s': %s", filename, strerror(errno));
	}

	if (fseek(f, 0, SEEK_SET) < 0) {
		fclose(f);
		js_error(J, "cannot seek in file '%s': %s", filename, strerror(errno));
	}

	s = malloc(n + 1);
	if (!s) {
		fclose(f);
		js_error(J, "out of memory");
	}

	t = fread(s, 1, n, f);
	if (t != n) {
		free(s);
		fclose(f);
		js_error(J, "cannot read data from file '%s': %s", filename, strerror(errno));
	}
	s[n] = 0;

	js_pushstring(J, s);
	free(s);
	fclose(f);
}

static void jsB_print(js_State *J) {
	int i, top = js_gettop(J);
	for (i = 1; i < top; i++) {
		const char *s = js_tostring(J, i);
	//	r_cons_printf ("%s\n", s);
	//	r_cons_flush ();
		if (i > 1) putchar(' ');
		fputs(s, stdout);
	}
	putchar('\n');
	js_pushundefined(J);
}

static void jsB_gc(js_State *J) {
	int report = js_toboolean(J, 1);
	js_gc(J, report);
	js_pushundefined(J);
}
