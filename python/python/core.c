/* radare - LGPL - Copyright 2016-2023 - pancake */

#include "core.h"

// XXX this is a global and it should die
R_TH_LOCAL RCore *core = NULL;

/* TODO : move into a struct stored in the plugin struct */
// XXX this is a global and it should die
static R_TH_LOCAL void *py_core_call_cb = NULL;

static int py_core_call(void *user, const char *str) {
	if (py_core_call_cb) {
		PyObject *arglist = Py_BuildValue ("(z)", str);
		PyObject *result = PyObject_CallObject (py_core_call_cb, arglist);
		const char * str_res = NULL;
		if (result) {
			if (PyLong_Check (result)) {
				return (int)PyLong_AsLong (result);
			} else if (PyLong_Check (result)) {
				return (int)PyLong_AsLong (result);
			} else if (PyUnicode_Check (result)) {
				int n = PyUnicode_KIND (result);
				switch (n) {
				case 1:
					str_res = (char*)PyUnicode_1BYTE_DATA (result);
					break;
				case 2:
					str_res = (char*)PyUnicode_2BYTE_DATA (result);
					break;
				case 4:
				default:
					str_res = (char*)PyUnicode_4BYTE_DATA (result);
					break;
				}
			} else
			if (PyUnicode_Check (result)) {
				str_res = PyBytes_AS_STRING (result);
			}
			if (str_res) {
				r_cons_print (str_res);
				return 1;
			}
		}
	}
	return 0;
}

void Radare_plugin_core_free(RCorePlugin *ap) {
	if (ap) {
#if R2_VERSION_NUMBER > 50808
		free ((char *)ap->meta.name);
		free ((char *)ap->meta.license);
		free ((char *)ap->meta.desc);
#else
		free ((char *)ap->name);
		free ((char *)ap->license);
		free ((char *)ap->desc);
#endif
		free (ap);
	}
}

PyObject *Radare_plugin_core(Radare* self, PyObject *args) {
	PyObject *arglist = Py_BuildValue("(i)", 0);
	PyObject *o = PyObject_CallObject (args, arglist);

	RCorePlugin *ap = R_NEW0 (RCorePlugin);
	if (!ap) {
		return NULL;
	}
#if R2_VERSION_NUMBER > 50808
	RPluginMeta meta = {
		.name = getS (o, "name"),
		.license = getS (o, "license"),
		.desc = getS (o, "desc")
	};
	memcpy ((RPluginMeta *)&ap->meta, &meta, sizeof (RPluginMeta));
#else
	ap->name = getS (o, "name");
	ap->license = getS (o, "license");
	ap->desc = getS (o, "desc");
#endif
	void *ptr = getF (o, "call");
	if (ptr) {
		Py_INCREF (ptr);
		py_core_call_cb = ptr;
		ap->call = py_core_call;
	}
	Py_DECREF (o);

	RLibStruct lp = {};
	lp.type = R_LIB_TYPE_CORE;
	lp.data = ap;
	lp.free = (void (*)(void *data))Radare_plugin_core_free;
	r_lib_open_ptr (core->lib, "python.py", NULL, &lp);
	Py_RETURN_TRUE;
}
