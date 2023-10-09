/* radare2 - LGPL - Copyright 2009-2023 - pancake */
/* python extension for radare2's r_lang */

#include <r_core.h>
#include "python/common.h"
#include "python/core.h"
#include "python/io.h"
#if R2_VERSION_NUMBER < 50809
#include "python/anal.h"
#include "python/asm.h"
#include "python/bin.h"
#endif

#define PLUGIN_NAME r_lang_plugin_python

typedef struct {
	const char *type;
	PyObject* (*handler)(Radare*, PyObject*);
} R2Plugins;

static const char *const py_nullstr = "";

static const R2Plugins plugins[] = {
	{ "core", &Radare_plugin_core },
#if R2_VERSION_NUMBER < 50800
	{ "asm", &Radare_plugin_asm },
#endif
#if R2_VERSION_NUMBER < 50809
	{ "anal", &Radare_plugin_anal },
	{ "bin", &Radare_plugin_bin },
#endif
	{ "io", &Radare_plugin_io },
	{ NULL }
};

static bool run(RLangSession *s, const char *code, int len) {
	RLang *lang = s->lang;
	core = (RCore *)lang->user;
	PyRun_SimpleString (code);
	return true;
}

static int slurp_python(const char *file) {
	FILE *fd = r_sandbox_fopen (file, "r");
	if (fd) {
		PyRun_SimpleFile (fd, file);
		fclose (fd);
		return true;
	}
	return false;
}

static bool run_file(RLangSession *session, const char *file) {
	return slurp_python (file);
}

static void Radare_dealloc(Radare* self) {
	if (self) {
		if (self->first) {
			Py_XDECREF (self->first);
		}
		if (self->last) {
			Py_XDECREF (self->last);
		}
	}
	// self->ob_type->tp_free((PyObject*)self);
}

static PyObject * Radare_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	Radare *self = (Radare *)type->tp_alloc (type, 0);
	if (self) {
		self->first = PyUnicode_FromString ("");
		if (!self->first) {
			Py_DECREF (self);
			return NULL;
		}
		self->last = PyUnicode_FromString ("");
		if (!self->last) {
			Py_DECREF (self);
			return NULL;
		}
		self->number = 0;
	}
	return (PyObject *)self;
}

static PyObject *Radare_plugin(Radare* self, PyObject *args) {
	char *type = NULL;
	void *cb = NULL;
	int i;

	if (!PyArg_ParseTuple (args, "sO", &type, &cb)) {
		return Py_False;
	}
	if (!PyCallable_Check (cb)) {
		PyErr_SetString (PyExc_TypeError, "second parameter must be callable");
		return Py_False;
	}
	for (i = 0; plugins[i].type; i++) {
		if (!strcmp (type, plugins[i].type)) {
			return plugins[i].handler (self, cb);
		}
	}
	R_LOG_INFO ("TODO: r2lang.plugin does not supports '%s' plugins yet", type);
	return Py_False;
}

static PyObject *Radare_print(Radare* self, PyObject *args) {
	char *cmd = NULL;
	if (!PyArg_ParseTuple (args, "s", &cmd)) {
		return Py_False;
	}
	r_cons_printf ("%s\n", cmd);
	return Py_True;
}

static PyObject *Radare_cmd(Radare* self, PyObject *args) {
	char *cmd = NULL;
	if (!PyArg_ParseTuple (args, "s", &cmd)) {
		return NULL;
	}
	char *str = r_core_cmd_str (core, cmd);
	return PyUnicode_FromString (str? str: py_nullstr);
}

static int Radare_init(Radare *self, PyObject *args, PyObject *kwds) {
	static char *kwlist[] = { "first", "last", "number", NULL };
	PyObject *first = NULL, *last = NULL, *tmp;

	if (!PyArg_ParseTupleAndKeywords (args, kwds, "|OOi",
			(char **)kwlist, &first, &last, &self->number)) {
		return -1;
	}
	if (first) {
		tmp = self->first;
		Py_INCREF (first);
		self->first = first;
		Py_XDECREF (tmp);
	}
	if (last) {
		tmp = self->last;
		Py_INCREF (last);
		self->last = last;
		Py_XDECREF (tmp);
	}
	return 0;
}

static PyMemberDef Radare_members[] = {
	{"first", T_OBJECT_EX, offsetof(Radare, first), 0, "first name"},
	{"last", T_OBJECT_EX, offsetof(Radare, last), 0, "last name"},
	{"number", T_INT, offsetof(Radare, number), 0, "noddy number"},
	{NULL}  /* Sentinel */
};

static PyMethodDef Radare_methods[] = {
	{"print", (PyCFunction)Radare_print, METH_VARARGS,
		"Print text using RCons api, so the output can be captured by radare2" },
	{"cmd", (PyCFunction)Radare_cmd, METH_VARARGS,
		"Executes a radare command and returns a string" },
	{"plugin", (PyCFunction)Radare_plugin, METH_VARARGS,
		"Register plugins in radare2" },
	{NULL}  /* Sentinel */
};

static PyTypeObject RadareType = {
	PyVarObject_HEAD_INIT (NULL, 0)
	"radare.RadareInternal",   /*tp_name*/
	sizeof (Radare),           /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Radare_dealloc,/*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"Radare objects",          /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	Radare_methods,            /* tp_methods */
	Radare_members,            /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)Radare_init,     /* tp_init */
	0,                         /* tp_alloc */
	Radare_new,                /* tp_new */
};

/*
SEE 
static PyMethodDef EmbMethods[] = {
    {"numargs", emb_numargs, METH_VARARGS,
     "Return the number of arguments received by the process."},
    {NULL, NULL, 0, NULL}
};
*/

static PyModuleDef EmbModule = {
	PyModuleDef_HEAD_INIT,
	"r2lang",
	NULL, -1, Radare_methods,
	NULL, NULL, NULL, NULL
};

static PyObject *init_radare_module(void) {
	// TODO import r2-swig api
	//eprintf ("TODO: python>3.x instantiate 'r' object\n");
	/* Gcore = lang->user; */
	/* eprintf("\x1b[31;1minit_radare_module\x1b[0m\n"); */
	if (PyType_Ready (&RadareType) < 0) {
		return NULL;
	}
	RadareType.tp_dict = PyDict_New ();
#if R2_VERSION_NUMBER < 50809
	py_export_anal_enum (RadareType.tp_dict);
#endif
#if R2_VERSION_NUMBER < 50800
	py_export_asm_enum (RadareType.tp_dict);
#endif
	PyObject *m = PyModule_Create (&EmbModule);
	if (!m) {
		R_LOG_ERROR ("Cannot create python3 r2 module");
		return NULL;
	}
	Py_INCREF (&RadareType);
	PyModule_AddObject (m, "R", (PyObject *)&RadareType);
	return m;
}

/* -init- */

static bool setup(RLangSession *user);

static bool prompt(RLangSession *s) {
	return !PyRun_SimpleString (
		"r2 = None\n"
		"have_ipy = False\n"
		"try:\n"
		"	import IPython\n"
		"	have_ipy = True\n"
		"except:\n"
		"	pass\n"
		"import r2lang\n"
		"import r2pipe\n"
		"r2 = r2pipe.open()\n"
		"if have_ipy:\n"
		"	IPython.embed()\n"
		"else:\n"
		"	import code\n"
		"	code.InteractiveConsole(locals=globals()).interact()\n"
	);
}

static bool setup(RLangSession *s) {
	RLang *lang = s->lang;
	RListIter *iter;
	RLangDef *def;
	char cmd[128];
	// Segfault if already initialized ?
	PyRun_SimpleString (
		"try:\n"
		"	from r2.r_core import RCore\n"
		"except:\n"
		"	pass\n");
	PyRun_SimpleString ("import r2pipe");
	core = lang->user;
	r_list_foreach (lang->defs, iter, def) {
		if (!def->type || !def->name) {
			continue;
		}
		if (!strcmp (def->type, "int"))
			snprintf (cmd, sizeof (cmd), "%s=%d", def->name, (int)(size_t)def->value);
		else if (!strcmp (def->type, "string"))
			snprintf (cmd, sizeof (cmd), "%s=\"%s\"", def->name, (char *)def->value);
		else snprintf (cmd, sizeof (cmd),
			"try:\n"
			"	%s=%s.ncast(%p)\n"
			"except:\n"
			"	pass", def->name, def->type, def->value);
		PyRun_SimpleString (cmd);
	}
	return true;
}

#if R2_VERSION_NUMBER > 50808
static bool init(RLangSession *session) {
	if (session == NULL) {
		return true;
	}
	RLang *lang = session->lang;
#elif R2_VERSION_NUMBER < 50800
static bool init(RLang *lang) {
#else
static void *init(RLangSession *session) {
	RLang *lang = session->lang;
#endif
	if (lang) {
		core = lang->user;
	}
	// DO NOT INITIALIZE MODULE IF ALREADY INITIALIZED
	if (Py_IsInitialized ()) {
		return NULL;
	}
	PyImport_AppendInittab ("r2lang", init_radare_module);
#if R2_VERSION_NUMBER < 50809
	PyImport_AppendInittab ("binfile", init_pybinfile_module);
#endif
	Py_Initialize ();
	// Add a current directory to the PYTHONPATH
	PyObject *sys = PyImport_ImportModule ("sys");
	PyObject *path = PyObject_GetAttrString (sys, "path");
	PyList_Append (path, PyUnicode_FromString("."));
#if R2_VERSION_NUMBER < 50809
	return sys;
#else
	session->plugin_data = sys;
	return true;
#endif
}

static bool fini(RLangSession *user) {
#if (PY_MAJOR_VERSION >= 3) && (PY_MINOR_VERSION >= 6)
	return Py_FinalizeEx () ? false : true;
#else
	Py_Finalize ();
	return true;
#endif
}

static const char *help =
	//" r = new RadareInternal()\n"
	"  print r2.cmd(\"p8 10\");\n";

RLangPlugin PLUGIN_NAME = {
#if R2_VERSION_NUMBER > 50808
	.meta = {
		.name = "python",
		.desc = "Python language extension",
		.license = "LGPL",
	},
#else
	.name = "python",
	.desc = "Python language extension",
	.license = "LGPL",
#endif
	.alias = "python",
	.ext = "py",
	.init = &init,
	.setup = &setup,
	.fini = (void *)&fini,
	.help = &help,
	.prompt = (void *)&prompt,
	.run = &run,
	.run_file = &run_file,
};

#if !CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &PLUGIN_NAME,
	.version = R2_VERSION
};
#endif
