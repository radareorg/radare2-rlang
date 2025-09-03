/* radare - LGPL - Copyright 2016-2025 - pancake */

#include "core.h"

// TODO: XXX remove globals
R_TH_LOCAL RCore *Gcore = NULL;
#if R2_VERSION_NUMBER > 50909
static R_TH_LOCAL void *py_core_call_cb = NULL;
#endif

#if R2_VERSION_NUMBER > 50909
static bool py_core_call(RCorePluginSession *s, const char *str);
#else
static int py_core_call(void *user, const char *str);
#endif

#if R2_VERSION_NUMBER > 50909
static bool py_core_call(RCorePluginSession *s, const char *str) {
	if (!Gcore) {
		return false;
	}
	if (!py_core_call_cb) {
		return false;
	}
	PyObject *arglist = Py_BuildValue ("(z)", str);
	PyObject *result = PyObject_CallObject ((PyObject *)py_core_call_cb, arglist);
	if (result) {
		int res = 0;
		if (PyBool_Check (result)) {
			if (result == Py_True) {
				Py_DECREF (result);
				Py_DECREF (arglist);
				return true;
			}
		} else if (PyLong_Check (result)) {
			res = (int)PyLong_AsLong (result);
			Py_DECREF (result);
			Py_DECREF (arglist);
			return res != 0;
		} else if (PyUnicode_Check (result)) {
			const char *resstr = PyBytes_AS_STRING (result);
			if (resstr != NULL) {
				r_cons_print (NULL, resstr);
				Py_DECREF (result);
				Py_DECREF (arglist);
				return true;
			}
		}
		Py_XDECREF (result);
	}
	Py_XDECREF (arglist);
	return false;
}
#else
static int py_core_call(void *user, const char *str) {
	RListIter *iter;
	RCorePlugin *cp;
	if (!Gcore) {
		return 0;
	}
	r_list_foreach (Gcore->rcmd->plist, iter, cp) {
		if (cp->data && cp->call == py_core_call) {
			void *py_core_call_cb = cp->data;
			PyObject *arglist = Py_BuildValue ("(z)", str);
			PyObject *result = PyObject_CallObject (py_core_call_cb, arglist);
			if (result) {
				int res = 0;
				if (PyBool_Check (result)) {
					if (result == Py_True) {
						return 1;
					}
				} else if (PyLong_Check (result)) {
					res = (int)PyLong_AsLong (result);
					if (res) {
						return res;
					}
				} else if (PyUnicode_Check (result)) {
					const char *resstr = PyBytes_AS_STRING (result);
					if (resstr != NULL) {
						r_cons_print (NULL, resstr);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
#endif

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
#if R2_VERSION_NUMBER > 50909
		py_core_call_cb = ptr;
#else
		ap->data = ptr;
#endif
		ap->call = py_core_call;
	}
	Py_DECREF (o);
	Py_DECREF (arglist);

	RLibStruct lp = {
		.type = R_LIB_TYPE_CORE,
		.data = ap,
		.free = (void (*)(void *data))Radare_plugin_core_free
	};
#if R2_VERSION_NUMBER > 50808
	R_LOG_DEBUG ("PLUGIN [python] Loading core: %s", ap->meta.name);
#else
	R_LOG_DEBUG ("PLUGIN [python] Loading core: %s", ap->name);
#endif
	r_lib_open_ptr (Gcore->lib, "python-r_core.py", NULL, &lp);
	Py_RETURN_TRUE;
}
