/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include "plugin.h"

static void tcl_iohack_free(void *p) {
	TclIOPlugin *plugin = (TclIOPlugin *)p;
	if (!plugin) {
		return;
	}
	tcl_obj_unref (plugin->check);
	tcl_obj_unref (plugin->open);
	tcl_obj_unref (plugin->read);
	tcl_obj_unref (plugin->seek);
	tcl_obj_unref (plugin->write);
	tcl_obj_unref (plugin->system);
	tcl_obj_unref (plugin->close);
	free (plugin->uris);
	free (plugin->name);
	free (plugin);
}

static TclIOPlugin *tcl_find_io_plugin(TclPluginContext *ctx, const char *name) {
	if (!ctx || !ctx->io_plugins || R_STR_ISEMPTY (name)) {
		return NULL;
	}
	RListIter *it;
	TclIOPlugin *plugin;
	r_list_foreach (ctx->io_plugins, it, plugin) {
		if (!strcmp (plugin->name, name)) {
			return plugin;
		}
	}
	return NULL;
}

static bool tcl_io_check(RIO *io, const char *path, bool many);

static bool tcl_io_check_internal(TclIOPlugin *plugin, const char *path, bool many) {
	Tcl_Obj *args[2] = {
		Tcl_NewStringObj (path? path: "", -1),
		Tcl_NewBooleanObj (many)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->check, 2, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin check error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	bool ok = false;
	tcl_obj_to_boolish (plugin->interp, result, &ok);
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return ok;
}

static RIOPlugin *tcl_io_lookup(RIO *io, const char *path, bool many) {
	if (!io) {
		return NULL;
	}
	SdbListIter *it;
	RIOPlugin *iop;
	ls_foreach (io->plugins, it, iop) {
		TclIOPlugin *plugin = iop? (TclIOPlugin *)iop->data: NULL;
		if (!plugin || iop->check != tcl_io_check) {
			continue;
		}
		if (iop->check && tcl_io_check_internal (plugin, path, many)) {
			return iop;
		}
	}
	return NULL;
}

static bool tcl_io_check(RIO *io, const char *path, bool many) {
	return tcl_io_lookup (io, path, many) != NULL;
}

static RIODesc *tcl_io_open(RIO *io, const char *path, int perm, int mode) {
	RIOPlugin *iop = tcl_io_lookup (io, path, false);
	if (!iop) {
		R_LOG_ERROR ("Cannot find TCL io plugin for %s", path);
		return NULL;
	}
	TclIOPlugin *plugin = (TclIOPlugin *)iop->data;
	Tcl_Obj *args[3] = {
		Tcl_NewStringObj (path? path: "", -1),
		Tcl_NewIntObj (perm),
		Tcl_NewIntObj (mode)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->open, 3, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin open error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return NULL;
	}
	bool ok = false;
	tcl_obj_to_boolish (plugin->interp, result, &ok);
	if (!ok) {
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return NULL;
	}
	TclIODescData *dd = R_NEW0 (TclIODescData);
	if (!dd) {
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return NULL;
	}
	dd->plugin = plugin;
	dd->state = result;
	return r_io_desc_new (io, iop, path, perm, mode, dd);
}

static int tcl_io_read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
	TclIODescData *dd = fd? (TclIODescData *)fd->data: NULL;
	if (!dd || !dd->plugin || !dd->plugin->read) {
		return -1;
	}
	Tcl_Obj *args[2] = {
		dd->state,
		Tcl_NewIntObj (count)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (dd->plugin->interp, dd->plugin->read, 2, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin read error: %s", Tcl_GetStringResult (dd->plugin->interp));
		tcl_reset_result (dd->plugin->interp);
		return -1;
	}
	memset (buf, io->Oxff, count);
	Tcl_Size objc = 0;
	Tcl_Obj **objv = NULL;
	if (Tcl_ListObjGetElements (dd->plugin->interp, result, &objc, &objv) == TCL_OK) {
		int i;
		for (i = 0; i < (int)objc && i < count; i++) {
			Tcl_WideInt value = 0;
			if (!tcl_obj_to_wide (dd->plugin->interp, objv[i], &value)) {
				objc = -1;
				break;
			}
			buf[i] = (ut8)(value & 0xff);
		}
		if (objc >= 0) {
			tcl_obj_unref (result);
			tcl_reset_result (dd->plugin->interp);
			return R_MIN ((int)objc, count);
		}
	}
	tcl_reset_result (dd->plugin->interp);
	Tcl_Size len = 0;
	unsigned char *bytes = Tcl_GetByteArrayFromObj (result, &len);
	if (bytes && len > 0) {
		const int copied = R_MIN ((int)len, count);
		memcpy (buf, bytes, copied);
		tcl_obj_unref (result);
		tcl_reset_result (dd->plugin->interp);
		return copied;
	}
	tcl_obj_unref (result);
	tcl_reset_result (dd->plugin->interp);
	return 0;
}

static ut64 tcl_io_seek(RIO *io, RIODesc *fd, ut64 offset, int whence) {
	TclIODescData *dd = fd? (TclIODescData *)fd->data: NULL;
	if (!dd || !dd->plugin || !dd->plugin->seek) {
		return UT64_MAX;
	}
	Tcl_Obj *args[3] = {
		dd->state,
		Tcl_NewWideIntObj ((Tcl_WideInt)offset),
		Tcl_NewIntObj (whence)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (dd->plugin->interp, dd->plugin->seek, 3, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin seek error: %s", Tcl_GetStringResult (dd->plugin->interp));
		tcl_reset_result (dd->plugin->interp);
		return UT64_MAX;
	}
	Tcl_WideInt value = 0;
	if (!tcl_obj_to_wide (dd->plugin->interp, result, &value)) {
		tcl_obj_unref (result);
		tcl_reset_result (dd->plugin->interp);
		return UT64_MAX;
	}
	dd->off = (ut64)value;
	tcl_obj_unref (result);
	tcl_reset_result (dd->plugin->interp);
	return dd->off;
}

static int tcl_io_write(RIO *io, RIODesc *fd, const ut8 *buf, int count) {
	TclIODescData *dd = fd? (TclIODescData *)fd->data: NULL;
	if (!dd || !dd->plugin || !dd->plugin->write) {
		return -1;
	}
	Tcl_Obj *args[2] = {
		dd->state,
		Tcl_NewByteArrayObj ((const unsigned char *)buf, count)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (dd->plugin->interp, dd->plugin->write, 2, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin write error: %s", Tcl_GetStringResult (dd->plugin->interp));
		tcl_reset_result (dd->plugin->interp);
		return -1;
	}
	Tcl_WideInt value = 0;
	if (tcl_obj_to_wide (dd->plugin->interp, result, &value)) {
		tcl_obj_unref (result);
		tcl_reset_result (dd->plugin->interp);
		return (int)value;
	}
	bool ok = false;
	tcl_obj_to_boolish (dd->plugin->interp, result, &ok);
	tcl_obj_unref (result);
	tcl_reset_result (dd->plugin->interp);
	return ok? count: -1;
}

static char *tcl_io_system(RIO *io, RIODesc *fd, const char *cmd) {
	TclIODescData *dd = fd? (TclIODescData *)fd->data: NULL;
	if (!dd || !dd->plugin || !dd->plugin->system || R_STR_ISEMPTY (cmd)) {
		return NULL;
	}
	Tcl_Obj *args[2] = {
		dd->state,
		Tcl_NewStringObj (cmd, -1)
	};
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (dd->plugin->interp, dd->plugin->system, 2, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL io plugin system error: %s", Tcl_GetStringResult (dd->plugin->interp));
		tcl_reset_result (dd->plugin->interp);
		return NULL;
	}
	const char *s = Tcl_GetString (result);
	char *out = s? strdup (s): NULL;
	tcl_obj_unref (result);
	tcl_reset_result (dd->plugin->interp);
	return out;
}

static bool tcl_io_close(RIODesc *fd) {
	TclIODescData *dd = fd? (TclIODescData *)fd->data: NULL;
	if (!dd) {
		return false;
	}
	bool ok = true;
	if (dd->plugin && dd->plugin->close) {
		Tcl_Obj *args[1] = { dd->state };
		Tcl_Obj *result = NULL;
		if (tcl_eval_prefix (dd->plugin->interp, dd->plugin->close, 1, args, &result) == TCL_OK) {
			tcl_obj_to_boolish (dd->plugin->interp, result, &ok);
			tcl_obj_unref (result);
		} else {
			R_LOG_ERROR ("TCL io plugin close error: %s", Tcl_GetStringResult (dd->plugin->interp));
			ok = false;
		}
		tcl_reset_result (dd->plugin->interp);
	}
	tcl_obj_unref (dd->state);
	R_FREE (fd->data);
	return ok;
}

static void tcl_io_plugin_free(RIOPlugin *plugin) {
	if (!plugin) {
		return;
	}
	free ((char *)plugin->meta.name);
	free ((char *)plugin->meta.desc);
	free ((char *)plugin->meta.license);
	free ((char *)plugin->uris);
	free (plugin);
}

int tcl_register_io_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec) {
	int ret = 0;
	char *name = tcl_dict_dup_string (interp, spec, "name", true);
	Tcl_Obj *check = tcl_dict_dup_callback (interp, spec, "check", true);
	Tcl_Obj *open = tcl_dict_dup_callback (interp, spec, "open", true);
	Tcl_Obj *read = tcl_dict_dup_callback (interp, spec, "read", true);
	Tcl_Obj *seek = tcl_dict_dup_callback (interp, spec, "seek", true);
	Tcl_Obj *write = tcl_dict_dup_callback (interp, spec, "write", false);
	Tcl_Obj *system = tcl_dict_dup_callback (interp, spec, "system", false);
	Tcl_Obj *close = tcl_dict_dup_callback (interp, spec, "close", false);
	char *desc = tcl_dict_dup_string (interp, spec, "desc", false);
	char *license = tcl_dict_dup_string (interp, spec, "license", false);
	char *uris = tcl_dict_dup_string (interp, spec, "uris", false);
	if (name && check && open && read && seek && !tcl_find_io_plugin (ctx, name)) {
		RIOPlugin *plugin = R_NEW0 (RIOPlugin);
		TclIOPlugin *hack = R_NEW0 (TclIOPlugin);
		RList *plugins = tcl_ensure_plugin_list (&ctx->io_plugins, tcl_iohack_free);
		if (plugins) {
			RPluginMeta meta = {
				.name = strdup (name),
				.desc = tcl_strdup0 (desc),
				.license = tcl_strdup0 (license),
			};
			memcpy ((void *)&plugin->meta, &meta, sizeof (meta));
			plugin->data = hack;
			plugin->uris = tcl_strdup0 (uris);
			plugin->open = tcl_io_open;
			plugin->check = tcl_io_check;
			plugin->read = tcl_io_read;
			plugin->seek = tcl_io_seek;
			plugin->write = write? tcl_io_write: NULL;
			plugin->system = system? tcl_io_system: NULL;
			plugin->close = close? tcl_io_close: NULL;
			hack->name = name;
			hack->interp = interp;
			hack->check = check;
			hack->open = open;
			hack->read = read;
			hack->seek = seek;
			hack->write = write;
			hack->system = system;
			hack->close = close;
			hack->uris = tcl_strdup0 (uris);
			ret = tcl_open_plugin (ctx, plugin->meta.name, R_LIB_TYPE_IO, plugin, (void (*)(void *))tcl_io_plugin_free);
			if (ret == 1) {
				r_list_append (plugins, hack);
				hack = NULL;
			}
		}
		if (hack) {
			tcl_iohack_free (hack);
		}
		if (!ret) {
			tcl_io_plugin_free (plugin);
		}
	}
	if (!ret) {
		tcl_obj_unref (check);
		tcl_obj_unref (open);
		tcl_obj_unref (read);
		tcl_obj_unref (seek);
		tcl_obj_unref (write);
		tcl_obj_unref (system);
		tcl_obj_unref (close);
		free (name);
	}
	free (desc);
	free (license);
	free (uris);
	return ret;
}
