/* radare - LGPL - Copyright 2009-2025 - pancake */

#include "io.h"
#include "core.h"

typedef struct {
	ut64 off;
	PyObject *result;
} DescData;

#if 0
/* r_io */
static RIOPlugin *py_io_plugin = NULL;
#else
static void *py_io_open_cb = NULL;
static void *py_io_check_cb = NULL;
static void *py_io_read_cb = NULL;
static void *py_io_system_cb = NULL;
static void *py_io_seek_cb = NULL;
static void *py_io_close_cb = NULL;
#endif

static bool py_io_check(RIO *io, const char *path, bool many);

static RIOPlugin *iop_check(RIO *io, const char *path) {
	SdbListIter *iter;
	RIOPlugin *iop;
	ls_foreach (io->plugins, iter, iop) {
		if (iop->check == py_io_check) {
			if (py_io_check (io, path, false)) {
				return iop;
			}
		}
	}
	return NULL;
}

static RIODesc* py_io_open(RIO *io, const char *path, int rw, int mode) {
	if (py_io_open_cb) {
		RIOPlugin *py_io_plugin = iop_check (io, path);
		if (!py_io_plugin) {
			return NULL;
		}
		PyObject *arglist = Py_BuildValue ("(zii)", path, rw, mode);
		PyObject *result = PyObject_CallObject (py_io_open_cb, arglist);
		if (!result) { // exception was thrown
			return NULL;
		}
		Py_DECREF (arglist);
		Py_INCREF (result);
		DescData *dd = R_NEW0 (DescData);
		dd->result = result;
		return r_io_desc_new (io, py_io_plugin, path, rw, mode, dd);
	}
	return NULL;
}

static bool py_io_check(RIO *io, const char *path, bool many) {
	bool res = false;
	if (py_io_check_cb) {
		PyObject *arglist = Py_BuildValue ("(zO)", path, many?Py_True:Py_False);
		Py_INCREF (arglist);
		PyObject *result = PyObject_CallObject (py_io_check_cb, arglist);
		if (result && PyBool_Check (result)) {
			res = result == Py_True;
			Py_DECREF (result);
		}
		Py_DECREF (arglist);
	}
	return res; 
}

static ut64 py_io_seek(RIO *io, RIODesc *fd, ut64 offset, int whence) {
	if (py_io_seek_cb) {
		DescData *dd = fd->data;
		PyObject *arglist = Py_BuildValue ("(NKi)", (PyObject *)dd->result, offset, whence);
		if (!arglist) {
			return UT64_MAX;
		}
		Py_INCREF (arglist);
		PyObject *result = PyObject_CallObject (py_io_seek_cb, arglist);
		if (result) {
			if (PyLong_Check (result)) {
				dd->off = PyLong_AsLong (result);
			} else if (PyLong_Check (result)) {
				dd->off = PyLong_AsLongLong (result);
			}
			return dd->off;
		} else {
			R_LOG_ERROR ("NaN");
		}
		// PyObject_Print (result, stderr, 0);
		// eprintf ("SEEK Unknown type returned. Number was expected.\n");
		switch (whence) {
		case 0: return dd->off = offset;
		case 1: return dd->off += offset;
		case 2: return 512; // wtf is this assumption
		}
		return UT64_MAX;
	}
	return UT64_MAX;
}

static int py_io_read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
	if (!py_io_read_cb) {
		return -1;
	}
	DescData *dd = fd->data;
	PyObject *arglist = Py_BuildValue ("(Oi)", (PyObject *)dd->result, count);
	Py_INCREF (arglist);
	PyObject *result = PyObject_CallObject (py_io_read_cb, arglist);
	if (result) {
		if (PyByteArray_Check (result)) {
			const char *ptr = PyByteArray_AsString (result);
			ssize_t size = PyByteArray_Size (result);
			ssize_t limit = R_MIN (size, (ssize_t)count);
			memset (buf, io->Oxff, limit);
			memcpy (buf, ptr, limit);
			count = (int)limit;
		} else if (PyUnicode_Check (result)) {
			//  PyObject* repr = PyObject_Repr(result);
			//  PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
			ssize_t size;
			const char *ptr = PyUnicode_AsUTF8AndSize (result, &size);
			ssize_t limit = R_MIN (size, (ssize_t)count);
			memset (buf, io->Oxff, limit);
			memcpy (buf, ptr, limit);
			count = (int)limit;
		} else if (PyBytes_Check (result)) {
			size_t size = PyBytes_Size (result);
			size_t limit = R_MIN (size, (size_t)count);
			memset (buf, io->Oxff, limit);
			memcpy (buf, PyBytes_AS_STRING (result), limit);
			// eprintf ("result is a string DONE %d %d\n" , count, size);
			count = (int)limit;
		} else if (PyList_Check (result)) {
			int i, size = PyList_Size (result);
			int limit = R_MIN (size, count);
			memset (buf, io->Oxff, count);
			for (i = 0; i < limit; i++) {
				PyObject *len = PyList_GetItem (result, i);
				buf[i] = PyNumber_AsSsize_t (len, NULL);
			}
			count = (int)limit;
		}
	} else {
		R_LOG_ERROR ("Unknown type returned. List was expected");
	}
	Py_DECREF (arglist);
	Py_DECREF (result);
	return count;
}

static char *py_io_system(RIO *io, RIODesc *desc, const char *cmd) {
	DescData *dd = desc->data;
	char * res = NULL;
	if (py_io_system_cb) {
		PyObject *arglist = Py_BuildValue ("(Oz)", (PyObject *)dd->result, cmd);
		Py_INCREF (arglist);
		PyObject *result = PyObject_CallObject (py_io_system_cb, arglist);
		if (result) {
			if (PyUnicode_Check (result)) {
				res = PyBytes_AS_STRING (result);
			} else if (PyBool_Check (result)) {
				res = strdup (r_str_bool (result == Py_True));
			} else if (PyLong_Check (result)) {
				long n = PyLong_AsLong (result);
				res = r_str_newf ("%ld", n);
			} else {
				R_LOG_ERROR ("Unknown type returned. Boolean was expected");
			}
		}
		// PyObject_Print(result, stderr, 0);
		Py_DECREF (arglist);
		Py_DECREF (result);
	}
	return res;
}

static bool py_io_close(RIODesc *desc) {
	DescData *dd = desc->data;
	int ret = 0;
	if (dd && py_io_close_cb) {
		PyObject *arglist = Py_BuildValue ("(N)", (PyObject *)dd->result);
		PyObject *result = PyObject_CallObject (py_io_close_cb, arglist);
		if (PyLong_Check (result)) {
			ret = PyLong_AsLong (result);
		}
		Py_DECREF (arglist);
		Py_DECREF (result);
		while (Py_REFCNT (dd->result)) { // HACK
			Py_DECREF (dd->result);
		}
		R_FREE (desc->data);
	}
	return ret != 0;
}

void Radare_plugin_io_free(RIOPlugin *ap) {
#if R2_VERSION_NUMBER > 50808
	free ((char *)ap->meta.name);
	free ((char *)ap->meta.desc);
	free ((char *)ap->meta.license);
#else
	free ((char *)ap->name);
	free ((char *)ap->desc);
	free ((char *)ap->license);
#endif
	free (ap);
}

PyObject *Radare_plugin_io(Radare* self, PyObject *args) {
	PyObject *arglist = Py_BuildValue("(i)", 0);
	PyObject *o = PyObject_CallObject (args, arglist);

	RIOPlugin *ap = R_NEW0 (RIOPlugin);
	if (!ap) {
		return Py_False;
	}
#if R2_VERSION_NUMBER > 50808
	RPluginMeta meta = {
		.name = getS (o, "name"),
		.desc = getS (o, "desc"),
		.license = getS (o, "license")
	};
	memcpy ((void*)&ap->meta, &meta, sizeof(RPluginMeta));
#if 0
	ap->meta.name = getS (o, "name");
	ap->meta.desc = getS (o, "desc");
	ap->meta.license = getS (o, "license");
#endif
#else
	ap->name = getS (o, "name");
	ap->desc = getS (o, "desc");
	ap->license = getS (o, "license");
#endif
	void *ptr = getF (o, "open");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_open_cb = (void *)ptr;
		ap->open = py_io_open;
	}
	ptr = getF (o, "check");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_check_cb = (void *)ptr;
		ap->check = py_io_check;
	}
	ptr = getF (o, "read");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_read_cb = (void *)ptr;
		ap->read = py_io_read;
	}
	ptr = getF (o, "system");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_system_cb = (void *)ptr;
		ap->system = py_io_system;
	}
	ptr = getF (o, "seek");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_seek_cb = (void *)ptr;
		ap->seek = py_io_seek;
	}
	ptr = getF (o, "close");
	if (ptr) {
		Py_INCREF (ptr);
		py_io_close_cb = (void *)ptr;
		ap->close = py_io_close;
	}
#if 0
	ptr = getF (o, "write");
	ptr = getF (o, "resize");
#endif
	Py_DECREF (o);

	RLibStruct lp = {};
	lp.type = R_LIB_TYPE_IO;
	lp.data = ap;
	lp.free = (void (*)(void *data))Radare_plugin_io_free;
	R_LOG_DEBUG ("PLUGIN[python] Loading io: %s", meta.name);
	r_lib_open_ptr (core->lib, "python.py", NULL, &lp);
	Py_RETURN_TRUE;
}

