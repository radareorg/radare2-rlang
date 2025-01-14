/* radare - LGPL - Copyright 2016-2025 - pancake */

#include "core.h"

// TODO: XXX remove globals
R_TH_LOCAL RCore *Gcore = NULL;
static R_TH_LOCAL void *py_core_call_cb = NULL;

static int py_core_call(void *user, const char *str) {
	if (py_core_call_cb == NULL) {
		return 0;
	}
	PyObject *arglist = Py_BuildValue ("(z)", str);
	PyObject *result = PyObject_CallObject (py_core_call_cb, arglist);
	if (result) {
		if (PyLong_Check (result)) {
			return (int)PyLong_AsLong (result);
		}
		if (PyUnicode_Check (result)) {
			const char *res = PyBytes_AS_STRING (result);
			if (res != NULL) {
				r_cons_print (res);
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
	PyObject *arglist = Py_BuildValue ("(i)", 0);
	PyObject *o = PyObject_CallObject (args, arglist);
	if (o == NULL) {
		return NULL;
	}

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
	Py_DECREF (arglist);

	RLibStruct lp = {
		.type = R_LIB_TYPE_CORE,
		.data = ap,
		.free = (void (*)(void *data))Radare_plugin_core_free
	};
	R_LOG_DEBUG ("PLUGIN[python] Loading core: %s", meta.name);
	r_lib_open_ptr (Gcore->lib, "python-r_core.py", NULL, &lp);
	Py_RETURN_TRUE;
}
