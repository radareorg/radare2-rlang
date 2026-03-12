/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include "plugin.h"

static void tcl_archhack_free(void *p) {
	TclArchPlugin *plugin = (TclArchPlugin *)p;
	if (!plugin) {
		return;
	}
	tcl_obj_unref (plugin->init);
	tcl_obj_unref (plugin->fini);
	tcl_obj_unref (plugin->info);
	tcl_obj_unref (plugin->regs);
	tcl_obj_unref (plugin->encode);
	tcl_obj_unref (plugin->decode);
	free (plugin->name);
	free (plugin->arch);
	free (plugin);
}

static TclArchPlugin *tcl_find_arch_plugin_by_name(TclPluginContext *ctx, const char *name) {
	if (!ctx || !ctx->arch_plugins || R_STR_ISEMPTY (name)) {
		return NULL;
	}
	RListIter *it;
	TclArchPlugin *plugin;
	r_list_foreach (ctx->arch_plugins, it, plugin) {
		if (!strcmp (plugin->name, name)) {
			return plugin;
		}
	}
	return NULL;
}

static TclArchPlugin *tcl_find_arch_plugin_by_arch(TclPluginContext *ctx, const char *arch) {
	if (!ctx || !ctx->arch_plugins || R_STR_ISEMPTY (arch)) {
		return NULL;
	}
	RListIter *it;
	TclArchPlugin *plugin;
	r_list_foreach (ctx->arch_plugins, it, plugin) {
		if (!strcmp (plugin->arch, arch)) {
			return plugin;
		}
	}
	return NULL;
}

static TclArchPlugin *tcl_arch_from_session(RArchSession *as) {
	if (!as) {
		return NULL;
	}
	if (as->data) {
		return (TclArchPlugin *)as->data;
	}
	RCore *core = (RCore *)as->user;
	TclPluginContext *ctx = tcl_context (core);
	if (!ctx) {
		return NULL;
	}
	TclArchPlugin *plugin = NULL;
	if (as->plugin && as->plugin->arch) {
		plugin = tcl_find_arch_plugin_by_arch (ctx, as->plugin->arch);
	}
	if (!plugin && as->plugin && as->plugin->meta.name) {
		plugin = tcl_find_arch_plugin_by_name (ctx, as->plugin->meta.name);
	}
	as->data = plugin;
	return plugin;
}

static const char *tcl_arch_info_name(ut32 query) {
	switch (query) {
	case R_ARCH_INFO_MINOP_SIZE: return "minop-size";
	case R_ARCH_INFO_MAXOP_SIZE: return "maxop-size";
	case R_ARCH_INFO_INVOP_SIZE: return "invop-size";
	case R_ARCH_INFO_CODE_ALIGN: return "code-align";
	case R_ARCH_INFO_DATA_ALIGN: return "data-align";
	case R_ARCH_INFO_DATA2_ALIGN: return "data2-align";
	case R_ARCH_INFO_DATA4_ALIGN: return "data4-align";
	case R_ARCH_INFO_DATA8_ALIGN: return "data8-align";
	default: return NULL;
	}
}

static int tcl_arch_parse_direction(const char *value) {
	if (R_STR_ISEMPTY (value)) {
		return -1;
	}
	if (!strcmp (value, "read")) {
		return R_ANAL_OP_DIR_READ;
	}
	if (!strcmp (value, "write")) {
		return R_ANAL_OP_DIR_WRITE;
	}
	if (!strcmp (value, "exec")) {
		return R_ANAL_OP_DIR_EXEC;
	}
	if (!strcmp (value, "ref")) {
		return R_ANAL_OP_DIR_REF;
	}
	return -1;
}

static int tcl_arch_parse_cond(const char *value) {
	if (R_STR_ISEMPTY (value)) {
		return R_ANAL_CONDTYPE_ERR;
	}
	if (!strcmp (value, "al")) {
		return R_ANAL_CONDTYPE_AL;
	}
	if (!strcmp (value, "eq")) {
		return R_ANAL_CONDTYPE_EQ;
	}
	if (!strcmp (value, "ne")) {
		return R_ANAL_CONDTYPE_NE;
	}
	if (!strcmp (value, "ge")) {
		return R_ANAL_CONDTYPE_GE;
	}
	if (!strcmp (value, "gt")) {
		return R_ANAL_CONDTYPE_GT;
	}
	if (!strcmp (value, "le")) {
		return R_ANAL_CONDTYPE_LE;
	}
	if (!strcmp (value, "lt")) {
		return R_ANAL_CONDTYPE_LT;
	}
	if (!strcmp (value, "nv")) {
		return R_ANAL_CONDTYPE_NV;
	}
	if (!strcmp (value, "hs")) {
		return R_ANAL_CONDTYPE_HS;
	}
	if (!strcmp (value, "lo")) {
		return R_ANAL_CONDTYPE_LO;
	}
	if (!strcmp (value, "mi")) {
		return R_ANAL_CONDTYPE_MI;
	}
	if (!strcmp (value, "pl")) {
		return R_ANAL_CONDTYPE_PL;
	}
	if (!strcmp (value, "vs")) {
		return R_ANAL_CONDTYPE_VS;
	}
	if (!strcmp (value, "vc")) {
		return R_ANAL_CONDTYPE_VC;
	}
	if (!strcmp (value, "hi")) {
		return R_ANAL_CONDTYPE_HI;
	}
	if (!strcmp (value, "ls")) {
		return R_ANAL_CONDTYPE_LS;
	}
	return R_ANAL_CONDTYPE_ERR;
}

static bool tcl_arch_set_numeric(Tcl_Interp *interp, Tcl_Obj *value, int *out) {
	Tcl_WideInt n = 0;
	if (tcl_obj_to_wide (interp, value, &n)) {
		*out = (int)n;
		return true;
	}
	return false;
}

static void tcl_arch_apply_string(RAnalOp *op, const char *key, const char *value) {
	if (!strcmp (key, "mnemonic")) {
		free (op->mnemonic);
		op->mnemonic = strdup (value);
	} else if (!strcmp (key, "esil")) {
		r_strbuf_set (&op->esil, value);
	} else if (!strcmp (key, "opex")) {
		r_strbuf_set (&op->opex, value);
	} else if (!strcmp (key, "reg")) {
		free ((void *)op->reg);
		op->reg = strdup (value);
	} else if (!strcmp (key, "ireg")) {
		free ((void *)op->ireg);
		op->ireg = strdup (value);
	} else if (!strcmp (key, "type")) {
		int n = r_anal_optype_from_string (value);
		if (n >= 0) {
			op->type = n;
		}
	} else if (!strcmp (key, "family")) {
		int n = r_anal_op_family_from_string (value);
		if (n >= 0) {
			op->family = n;
		}
	} else if (!strcmp (key, "direction")) {
		int n = tcl_arch_parse_direction (value);
		if (n >= 0) {
			op->direction = n;
		}
	} else if (!strcmp (key, "cond")) {
		int n = tcl_arch_parse_cond (value);
		if (n >= 0) {
			op->cond = n;
		}
	}
}

static void tcl_arch_apply_field(Tcl_Interp *interp, RAnalOp *op, Tcl_Obj *keyObj, Tcl_Obj *valueObj) {
	const char *key = Tcl_GetString (keyObj);
	const char *str = Tcl_GetString (valueObj);
	if (!strcmp (key, "mnemonic") || !strcmp (key, "esil") || !strcmp (key, "opex") || !strcmp (key, "reg") || !strcmp (key, "ireg")) {
		tcl_arch_apply_string (op, key, str);
		return;
	}
	if (!strcmp (key, "type") || !strcmp (key, "family") || !strcmp (key, "direction") || !strcmp (key, "cond")) {
		int n = 0;
		if (tcl_arch_set_numeric (interp, valueObj, &n)) {
			if (!strcmp (key, "type")) {
				op->type = n;
			} else if (!strcmp (key, "family")) {
				op->family = n;
			} else if (!strcmp (key, "direction")) {
				op->direction = n;
			} else {
				op->cond = n;
			}
			return;
		}
		tcl_arch_apply_string (op, key, str);
		return;
	}
	int n = 0;
	bool b = false;
	if (!strcmp (key, "size") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->size = n;
	} else if (!strcmp (key, "prefix") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->prefix = n;
	} else if (!strcmp (key, "type2") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->type2 = n;
	} else if (!strcmp (key, "stackop") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->stackop = n;
	} else if (!strcmp (key, "nopcode") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->nopcode = n;
	} else if (!strcmp (key, "cycles") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->cycles = n;
	} else if (!strcmp (key, "failcycles") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->failcycles = n;
	} else if (!strcmp (key, "id") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->id = n;
	} else if (!strcmp (key, "delay") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->delay = n;
	} else if (!strcmp (key, "ptrsize") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->ptrsize = n;
	} else if (!strcmp (key, "refptr") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->refptr = n;
	} else if (!strcmp (key, "scale") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->scale = n;
	} else if (!strcmp (key, "datatype") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->datatype = n;
	} else if (!strcmp (key, "vliw") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->vliw = n;
	} else if (!strcmp (key, "payload") && tcl_arch_set_numeric (interp, valueObj, &n)) {
		op->payload = n;
	} else if (!strcmp (key, "eob") && tcl_obj_to_boolish (interp, valueObj, &b)) {
		op->eob = b;
	} else if (!strcmp (key, "sign") && tcl_obj_to_boolish (interp, valueObj, &b)) {
		op->sign = b;
	} else if (!strcmp (key, "jump")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->jump = (ut64)w;
		}
	} else if (!strcmp (key, "fail")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->fail = (ut64)w;
		}
	} else if (!strcmp (key, "ptr")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->ptr = (st64)w;
		}
	} else if (!strcmp (key, "val")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->val = (ut64)w;
		}
	} else if (!strcmp (key, "stackptr")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->stackptr = (st64)w;
		}
	} else if (!strcmp (key, "disp")) {
		Tcl_WideInt w = 0;
		if (tcl_obj_to_wide (interp, valueObj, &w)) {
			op->disp = (ut64)w;
		}
	}
}

static bool tcl_arch_init(RArchSession *as) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin) {
		return false;
	}
	if (!plugin->init) {
		return true;
	}
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->init, 0, NULL, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL arch init error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	bool ok = true;
	tcl_obj_to_boolish (plugin->interp, result, &ok);
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return ok;
}

static bool tcl_arch_fini(RArchSession *as) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin || !plugin->fini) {
		return true;
	}
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->fini, 0, NULL, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL arch fini error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	bool ok = true;
	tcl_obj_to_boolish (plugin->interp, result, &ok);
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return ok;
}

static int tcl_arch_info(RArchSession *as, ut32 query) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin || !plugin->info) {
		return 0;
	}
	const char *name = tcl_arch_info_name (query);
	Tcl_Obj *result = NULL;
	Tcl_Obj *value = NULL;
	Tcl_Size dictSize = 0;
	if (Tcl_DictObjSize (plugin->interp, plugin->info, &dictSize) == TCL_OK) {
		if (name && tcl_get_dict_value (plugin->interp, plugin->info, name, &value) == TCL_OK && value) {
			result = tcl_dup_obj (value);
		} else {
			tcl_reset_result (plugin->interp);
			return 0;
		}
	} else {
		tcl_reset_result (plugin->interp);
		char numbuf[64];
		if (!name) {
			snprintf (numbuf, sizeof (numbuf), "%u", query);
		}
		Tcl_Obj *args[1] = {
			Tcl_NewStringObj (name? name: numbuf, -1)
		};
		if (tcl_eval_prefix (plugin->interp, plugin->info, 1, args, &result) != TCL_OK) {
			R_LOG_ERROR ("TCL arch info error: %s", Tcl_GetStringResult (plugin->interp));
			tcl_reset_result (plugin->interp);
			return 0;
		}
	}
	Tcl_WideInt out = 0;
	const bool ok = tcl_obj_to_wide (plugin->interp, result, &out);
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return ok? (int)out: 0;
}

static char *tcl_arch_regs(RArchSession *as) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin || !plugin->regs) {
		return NULL;
	}
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->regs, 0, NULL, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL arch regs error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return NULL;
	}
	const char *regs = Tcl_GetString (result);
	char *out = regs? strdup (regs): NULL;
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return out;
}

static bool tcl_arch_decode(RArchSession *as, RAnalOp *op, RArchDecodeMask mask) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin || !plugin->decode) {
		return false;
	}
	Tcl_Obj *arg = Tcl_NewDictObj ();
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("addr", -1), Tcl_NewWideIntObj ((Tcl_WideInt)op->addr));
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("size", -1), Tcl_NewIntObj (op->size));
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("mask", -1), Tcl_NewIntObj (mask));
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("bytes", -1), Tcl_NewByteArrayObj ((const unsigned char *)op->bytes, op->size));
	Tcl_Obj *args[1] = { arg };
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->decode, 1, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL arch decode error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	bool ok = false;
	tcl_obj_to_boolish (plugin->interp, result, &ok);
	if (!ok) {
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return false;
	}
	Tcl_Obj *dictObj = result;
	Tcl_Size objc = 0;
	Tcl_Obj **objv = NULL;
	if (Tcl_ListObjGetElements (plugin->interp, result, &objc, &objv) == TCL_OK && objc == 2) {
		Tcl_WideInt n = 0;
		if (tcl_obj_to_wide (plugin->interp, objv[0], &n)) {
			op->size = (int)n;
			dictObj = objv[1];
		}
	}
	tcl_reset_result (plugin->interp);
	Tcl_DictSearch search;
	Tcl_Obj *keyObj = NULL;
	Tcl_Obj *valueObj = NULL;
	int done = 0;
	if (Tcl_DictObjFirst (plugin->interp, dictObj, &search, &keyObj, &valueObj, &done) == TCL_OK) {
		while (!done) {
			tcl_arch_apply_field (plugin->interp, op, keyObj, valueObj);
			Tcl_DictObjNext (&search, &keyObj, &valueObj, &done);
		}
		Tcl_DictObjDone (&search);
	}
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return true;
}

static bool tcl_arch_encode(RArchSession *as, RAnalOp *op, RArchEncodeMask mask) {
	TclArchPlugin *plugin = tcl_arch_from_session (as);
	if (!plugin || !plugin->encode) {
		return false;
	}
	Tcl_Obj *arg = Tcl_NewDictObj ();
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("addr", -1), Tcl_NewWideIntObj ((Tcl_WideInt)op->addr));
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("mask", -1), Tcl_NewIntObj (mask));
	Tcl_DictObjPut (plugin->interp, arg, Tcl_NewStringObj ("mnemonic", -1), Tcl_NewStringObj (r_str_get (op->mnemonic), -1));
	Tcl_Obj *args[1] = { arg };
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (plugin->interp, plugin->encode, 1, args, &result) != TCL_OK) {
		R_LOG_ERROR ("TCL arch encode error: %s", Tcl_GetStringResult (plugin->interp));
		tcl_reset_result (plugin->interp);
		return false;
	}
	Tcl_Size size = 0;
	unsigned char *bytes = Tcl_GetByteArrayFromObj (result, &size);
	if (!bytes || size <= 0) {
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return false;
	}
	free (op->bytes);
	op->bytes = r_mem_dup (bytes, size);
	op->size = (int)size;
	tcl_obj_unref (result);
	tcl_reset_result (plugin->interp);
	return true;
}

static void tcl_arch_plugin_free(RArchPlugin *plugin) {
	if (!plugin) {
		return;
	}
	free (plugin->meta.name);
	free (plugin->meta.desc);
	free (plugin->meta.author);
	free (plugin->meta.license);
	free (plugin->meta.version);
	free (plugin->arch);
	free (plugin->cpus);
	free (plugin);
}

int tcl_register_arch_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec) {
	int ret = 0;
	char *name = tcl_dict_dup_string (interp, spec, "name", true);
	char *arch = tcl_dict_dup_string (interp, spec, "arch", true);
	char *desc = tcl_dict_dup_string (interp, spec, "desc", false);
	char *license = tcl_dict_dup_string (interp, spec, "license", false);
	char *author = tcl_dict_dup_string (interp, spec, "author", false);
	char *version = tcl_dict_dup_string (interp, spec, "version", false);
	char *cpus = tcl_dict_dup_string (interp, spec, "cpus", false);
	Tcl_Obj *decode = tcl_dict_dup_callback (interp, spec, "decode", true);
	Tcl_Obj *encode = tcl_dict_dup_callback (interp, spec, "encode", false);
	Tcl_Obj *initCb = tcl_dict_dup_callback (interp, spec, "init", false);
	Tcl_Obj *finiCb = tcl_dict_dup_callback (interp, spec, "fini", false);
	Tcl_Obj *info = tcl_dict_dup_callback (interp, spec, "info", false);
	if (!info) {
		Tcl_Obj *infoDict = NULL;
		if (tcl_get_dict_value (interp, spec, "info", &infoDict) == TCL_OK && infoDict) {
			info = tcl_dup_obj (infoDict);
		}
		tcl_reset_result (interp);
	}
	Tcl_Obj *regs = tcl_dict_dup_callback (interp, spec, "regs", false);
	ut32 bits = tcl_dict_get_u32 (interp, spec, "bits");
	ut32 endian = tcl_dict_get_endian (interp, spec, "endian");
	ut32 addr_bits = tcl_dict_get_u32 (interp, spec, "addr_bits");
	if (name && arch && decode && bits && !tcl_find_arch_plugin_by_name (ctx, name) && !tcl_find_arch_plugin_by_arch (ctx, arch)) {
		RArchPlugin *plugin = R_NEW0 (RArchPlugin);
		TclArchPlugin *hack = R_NEW0 (TclArchPlugin);
		RList *plugins = tcl_ensure_plugin_list (&ctx->arch_plugins, tcl_archhack_free);
		if (plugins) {
			plugin->meta.name = strdup (name);
			plugin->meta.desc = tcl_strdup0 (desc);
			plugin->meta.author = tcl_strdup0 (author);
			plugin->meta.license = tcl_strdup0 (license);
			plugin->meta.version = tcl_strdup0 (version);
			plugin->arch = strdup (arch);
			plugin->cpus = tcl_strdup0 (cpus);
			plugin->bits = bits;
			plugin->endian = endian;
			plugin->addr_bits = addr_bits;
			*((RArchPluginInitCallback *)&plugin->init) = tcl_arch_init;
			*((RArchPluginFiniCallback *)&plugin->fini) = tcl_arch_fini;
			*((RArchPluginInfoCallback *)&plugin->info) = info? tcl_arch_info: NULL;
			*((RArchPluginRegistersCallback *)&plugin->regs) = regs? tcl_arch_regs: NULL;
			*((RArchPluginEncodeCallback *)&plugin->encode) = encode? tcl_arch_encode: NULL;
			*((RArchPluginDecodeCallback *)&plugin->decode) = tcl_arch_decode;
			hack->name = name;
			hack->arch = arch;
			hack->interp = interp;
			hack->init = initCb;
			hack->fini = finiCb;
			hack->info = info;
			hack->regs = regs;
			hack->encode = encode;
			hack->decode = decode;
			ret = tcl_open_plugin (ctx, plugin->meta.name, R_LIB_TYPE_ARCH, plugin, (void (*)(void *))tcl_arch_plugin_free);
			if (ret == 1) {
				r_list_append (plugins, hack);
				hack = NULL;
			}
		}
		if (hack) {
			tcl_archhack_free (hack);
		}
		if (!ret) {
			tcl_arch_plugin_free (plugin);
		}
	}
	if (!ret) {
		tcl_obj_unref (decode);
		tcl_obj_unref (encode);
		tcl_obj_unref (initCb);
		tcl_obj_unref (finiCb);
		tcl_obj_unref (info);
		tcl_obj_unref (regs);
		free (name);
		free (arch);
	}
	free (desc);
	free (license);
	free (author);
	free (version);
	free (cpus);
	return ret;
}
