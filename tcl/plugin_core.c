/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include "plugin.h"

static void tcl_corehack_free(void *p) {
	TclCorePlugin *plugin = (TclCorePlugin *)p;
	if (!plugin) {
		return;
	}
	tcl_obj_unref (plugin->call);
	free (plugin->name);
	free (plugin);
}

static TclCorePlugin *tcl_find_core_plugin(TclPluginContext *ctx, const char *name) {
	if (!ctx || !ctx->core_plugins || R_STR_ISEMPTY (name)) {
		return NULL;
	}
	RListIter *it;
	TclCorePlugin *plugin;
	r_list_foreach (ctx->core_plugins, it, plugin) {
		if (!strcmp (plugin->name, name)) {
			return plugin;
		}
	}
	return NULL;
}

static bool tcl_core_init(RCorePluginSession *cps) {
	return cps != NULL;
}

static bool tcl_core_fini(RCorePluginSession *cps) {
	return cps != NULL;
}

static bool tcl_core_call(RCorePluginSession *cps, const char *input) {
	if (!cps || !cps->data) {
		if (!cps || !cps->plugin || !cps->plugin->meta.name) {
			return false;
		}
		TclPluginContext *ctx = tcl_context (cps->core);
		if (!ctx) {
			return false;
		}
		cps->data = tcl_find_core_plugin (ctx, cps->plugin->meta.name);
		if (!cps->data) {
			return false;
		}
	}
	TclCorePlugin *plugin = (TclCorePlugin *)cps->data;
	Tcl_Obj *args[1] = { Tcl_NewStringObj (input? input: "", -1) };
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->call, 1, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL core plugin error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	bool handled = false;
	int b = 0;
	if (Tcl_GetBooleanFromObj (plugin->interp, result, &b) == TCL_OK) {
		handled = b != 0;
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return handled;
	}
	Tcl_WideInt n = 0;
	tcl_reset_result (plugin->interp);
	if (Tcl_GetWideIntFromObj (plugin->interp, result, &n) == TCL_OK) {
		handled = n != 0;
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return handled;
	}
	tcl_reset_result (plugin->interp);
	const char *s = Tcl_GetString (result);
	if (R_STR_ISNOTEMPTY (s)) {
		r_cons_print (cps->core? cps->core->cons: NULL, s);
		r_cons_print (cps->core? cps->core->cons: NULL, "\n");
		handled = true;
	}
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return handled;
}

static void tcl_core_plugin_free(RCorePlugin *plugin) {
	if (!plugin) {
		return;
	}
	free ((char *)plugin->meta.name);
	free ((char *)plugin->meta.license);
	free ((char *)plugin->meta.desc);
	free (plugin);
}

int tcl_register_core_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec) {
	int ret = 0;
	char *name = tcl_dict_dup_string (interp, spec, "name", true);
	Tcl_Obj *call = tcl_dict_dup_callback (interp, spec, "call", true);
	char *desc = tcl_dict_dup_string (interp, spec, "desc", false);
	char *license = tcl_dict_dup_string (interp, spec, "license", false);
	if (name && call && !tcl_find_core_plugin (ctx, name)) {
		RCorePlugin *plugin = R_NEW0 (RCorePlugin);
		TclCorePlugin *hack = R_NEW0 (TclCorePlugin);
		RList *plugins = tcl_ensure_plugin_list (&ctx->core_plugins, tcl_corehack_free);
		if (plugins) {
			plugin->meta.name = strdup (name);
			plugin->meta.desc = tcl_strdup0 (desc);
			plugin->meta.license = tcl_strdup0 (license);
			plugin->init = tcl_core_init;
			plugin->fini = tcl_core_fini;
			plugin->call = tcl_core_call;
			hack->name = name;
			hack->interp = interp;
			hack->call = call;
			r_list_append (plugins, hack);
			ret = tcl_open_plugin (ctx, plugin->meta.name, R_LIB_TYPE_CORE, plugin, (void (*)(void *))tcl_core_plugin_free);
			if (ret == 1) {
				hack = NULL;
			} else {
				r_list_delete_data (plugins, hack);
			}
		}
		tcl_corehack_free (hack);
		if (!ret) {
			tcl_core_plugin_free (plugin);
		}
	}
	if (!ret) {
		tcl_obj_unref (call);
		free (name);
	}
	free (desc);
	free (license);
	return ret;
}
