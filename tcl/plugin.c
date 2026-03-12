/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include <r_arch.h>
#include <r_core.h>
#include <r_lang.h>
#include <r_lib.h>
#include <r_util.h>
#include <tcl.h>

typedef struct {
	const char *name;
	Tcl_WideInt value;
} TclNamedConstant;

typedef struct {
	char *name;
	Tcl_Interp *interp;
	Tcl_Obj *call;
} TclCorePlugin;

typedef struct {
	char *name;
	Tcl_Interp *interp;
	Tcl_Obj *check;
	Tcl_Obj *open;
	Tcl_Obj *read;
	Tcl_Obj *seek;
	Tcl_Obj *write;
	Tcl_Obj *system;
	Tcl_Obj *close;
	char *uris;
} TclIOPlugin;

typedef struct {
	TclIOPlugin *plugin;
	Tcl_Obj *state;
	ut64 off;
} TclIODescData;

typedef struct {
	char *name;
	char *arch;
	Tcl_Interp *interp;
	Tcl_Obj *init;
	Tcl_Obj *fini;
	Tcl_Obj *info;
	Tcl_Obj *regs;
	Tcl_Obj *encode;
	Tcl_Obj *decode;
} TclArchPlugin;

typedef struct {
	RCore *core;
	Tcl_Interp *interp;
	RList *core_plugins;
	RList *io_plugins;
	RList *arch_plugins;
} TclPluginContext;

#define TCL_CONSTANT(name) { #name, (Tcl_WideInt)(name) }

static const TclNamedConstant tcl_constants[] = {
	TCL_CONSTANT (R_IO_SEEK_SET),
	TCL_CONSTANT (R_IO_SEEK_CUR),
	TCL_CONSTANT (R_IO_SEEK_END),
	TCL_CONSTANT (R_ARCH_INFO_MINOP_SIZE),
	TCL_CONSTANT (R_ARCH_INFO_MAXOP_SIZE),
	TCL_CONSTANT (R_ARCH_INFO_INVOP_SIZE),
	TCL_CONSTANT (R_ARCH_INFO_CODE_ALIGN),
	TCL_CONSTANT (R_ARCH_INFO_DATA_ALIGN),
	TCL_CONSTANT (R_ARCH_INFO_DATA2_ALIGN),
	TCL_CONSTANT (R_ARCH_INFO_DATA4_ALIGN),
	TCL_CONSTANT (R_ARCH_INFO_DATA8_ALIGN),
	TCL_CONSTANT (R_ARCH_OP_MASK_BASIC),
	TCL_CONSTANT (R_ARCH_OP_MASK_ESIL),
	TCL_CONSTANT (R_ARCH_OP_MASK_VAL),
	TCL_CONSTANT (R_ARCH_OP_MASK_HINT),
	TCL_CONSTANT (R_ARCH_OP_MASK_OPEX),
	TCL_CONSTANT (R_ARCH_OP_MASK_DISASM),
	TCL_CONSTANT (R_ARCH_OP_MASK_ALL),
	TCL_CONSTANT (R_ARCH_OP_MOD_COND),
	TCL_CONSTANT (R_ARCH_OP_MOD_REP),
	TCL_CONSTANT (R_ARCH_OP_MOD_MEM),
	TCL_CONSTANT (R_ARCH_OP_MOD_REG),
	TCL_CONSTANT (R_ARCH_OP_MOD_IND),
	TCL_CONSTANT (R_ANAL_OP_TYPE_NULL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_JMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_RJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UCJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_IJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_IRJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_MJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_RCJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_MCJMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UCALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_RCALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ICALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_IRCALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CCALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UCCALL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_RET),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CRET),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ILL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UNK),
	TCL_CONSTANT (R_ANAL_OP_TYPE_NOP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_MOV),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CMOV),
	TCL_CONSTANT (R_ANAL_OP_TYPE_TRAP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SWI),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CSWI),
	TCL_CONSTANT (R_ANAL_OP_TYPE_UPUSH),
	TCL_CONSTANT (R_ANAL_OP_TYPE_RPUSH),
	TCL_CONSTANT (R_ANAL_OP_TYPE_PUSH),
	TCL_CONSTANT (R_ANAL_OP_TYPE_POP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ACMP),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ADD),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SUB),
	TCL_CONSTANT (R_ANAL_OP_TYPE_IO),
	TCL_CONSTANT (R_ANAL_OP_TYPE_MUL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_DIV),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SHR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SHL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SAL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SAR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_OR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_AND),
	TCL_CONSTANT (R_ANAL_OP_TYPE_XOR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_NOR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_NOT),
	TCL_CONSTANT (R_ANAL_OP_TYPE_STORE),
	TCL_CONSTANT (R_ANAL_OP_TYPE_LOAD),
	TCL_CONSTANT (R_ANAL_OP_TYPE_LEA),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ULEA),
	TCL_CONSTANT (R_ANAL_OP_TYPE_LEAVE),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ROR),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ROL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_XCHG),
	TCL_CONSTANT (R_ANAL_OP_TYPE_MOD),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SWITCH),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CASE),
	TCL_CONSTANT (R_ANAL_OP_TYPE_LENGTH),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CAST),
	TCL_CONSTANT (R_ANAL_OP_TYPE_NEW),
	TCL_CONSTANT (R_ANAL_OP_TYPE_ABS),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CPL),
	TCL_CONSTANT (R_ANAL_OP_TYPE_CRYPTO),
	TCL_CONSTANT (R_ANAL_OP_TYPE_SYNC),
	TCL_CONSTANT (R_ANAL_OP_TYPE_DEBUG),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_COND),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_REP),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_REPNE),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_LOCK),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_LIKELY),
	TCL_CONSTANT (R_ANAL_OP_PREFIX_UNLIKELY),
	TCL_CONSTANT (R_ANAL_STACK_NULL),
	TCL_CONSTANT (R_ANAL_STACK_NOP),
	TCL_CONSTANT (R_ANAL_STACK_INC),
	TCL_CONSTANT (R_ANAL_STACK_GET),
	TCL_CONSTANT (R_ANAL_STACK_SET),
	TCL_CONSTANT (R_ANAL_STACK_RESET),
	TCL_CONSTANT (R_ANAL_STACK_ALIGN),
	TCL_CONSTANT (R_ANAL_CONDTYPE_AL),
	TCL_CONSTANT (R_ANAL_CONDTYPE_EQ),
	TCL_CONSTANT (R_ANAL_CONDTYPE_NE),
	TCL_CONSTANT (R_ANAL_CONDTYPE_GE),
	TCL_CONSTANT (R_ANAL_CONDTYPE_GT),
	TCL_CONSTANT (R_ANAL_CONDTYPE_LE),
	TCL_CONSTANT (R_ANAL_CONDTYPE_LT),
	TCL_CONSTANT (R_ANAL_CONDTYPE_NV),
	TCL_CONSTANT (R_ANAL_CONDTYPE_HS),
	TCL_CONSTANT (R_ANAL_CONDTYPE_LO),
	TCL_CONSTANT (R_ANAL_CONDTYPE_MI),
	TCL_CONSTANT (R_ANAL_CONDTYPE_PL),
	TCL_CONSTANT (R_ANAL_CONDTYPE_VS),
	TCL_CONSTANT (R_ANAL_CONDTYPE_VC),
	TCL_CONSTANT (R_ANAL_CONDTYPE_HI),
	TCL_CONSTANT (R_ANAL_CONDTYPE_LS),
	TCL_CONSTANT (R_ANAL_OP_DIR_READ),
	TCL_CONSTANT (R_ANAL_OP_DIR_WRITE),
	TCL_CONSTANT (R_ANAL_OP_DIR_EXEC),
	TCL_CONSTANT (R_ANAL_OP_DIR_REF),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_UNKNOWN),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_CPU),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_FPU),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_VEC),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_PRIV),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_CRYPTO),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_THREAD),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_VIRT),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_SECURITY),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_IO),
	TCL_CONSTANT (R_ANAL_OP_FAMILY_SIMD),
	TCL_CONSTANT (R_ANAL_DATATYPE_NULL),
	TCL_CONSTANT (R_ANAL_DATATYPE_ARRAY),
	TCL_CONSTANT (R_ANAL_DATATYPE_OBJECT),
	TCL_CONSTANT (R_ANAL_DATATYPE_STRING),
	TCL_CONSTANT (R_ANAL_DATATYPE_CLASS),
	TCL_CONSTANT (R_ANAL_DATATYPE_BOOLEAN),
	TCL_CONSTANT (R_ANAL_DATATYPE_INT16),
	TCL_CONSTANT (R_ANAL_DATATYPE_INT32),
	TCL_CONSTANT (R_ANAL_DATATYPE_INT64),
	TCL_CONSTANT (R_ANAL_DATATYPE_FLOAT),
	{ "R_ANAL_COND_AL", (Tcl_WideInt)R_ANAL_CONDTYPE_AL },
	{ "R_ANAL_COND_EQ", (Tcl_WideInt)R_ANAL_CONDTYPE_EQ },
	{ "R_ANAL_COND_NE", (Tcl_WideInt)R_ANAL_CONDTYPE_NE },
	{ "R_ANAL_COND_GE", (Tcl_WideInt)R_ANAL_CONDTYPE_GE },
	{ "R_ANAL_COND_GT", (Tcl_WideInt)R_ANAL_CONDTYPE_GT },
	{ "R_ANAL_COND_LE", (Tcl_WideInt)R_ANAL_CONDTYPE_LE },
	{ "R_ANAL_COND_LT", (Tcl_WideInt)R_ANAL_CONDTYPE_LT },
	{ "R_ANAL_COND_NV", (Tcl_WideInt)R_ANAL_CONDTYPE_NV },
	{ "R_ANAL_COND_HS", (Tcl_WideInt)R_ANAL_CONDTYPE_HS },
	{ "R_ANAL_COND_LO", (Tcl_WideInt)R_ANAL_CONDTYPE_LO },
	{ "R_ANAL_COND_MI", (Tcl_WideInt)R_ANAL_CONDTYPE_MI },
	{ "R_ANAL_COND_PL", (Tcl_WideInt)R_ANAL_CONDTYPE_PL },
	{ "R_ANAL_COND_VS", (Tcl_WideInt)R_ANAL_CONDTYPE_VS },
	{ "R_ANAL_COND_VC", (Tcl_WideInt)R_ANAL_CONDTYPE_VC },
	{ "R_ANAL_COND_HI", (Tcl_WideInt)R_ANAL_CONDTYPE_HI },
	{ "R_ANAL_COND_LS", (Tcl_WideInt)R_ANAL_CONDTYPE_LS },
	{ NULL, 0 }
};

static void tcl_free(char *blockPtr) {
	free (blockPtr);
}

static void tcl_reset_result(Tcl_Interp *interp) {
	if (interp) {
		Tcl_ResetResult (interp);
	}
}

static int tcl_named_constant(const char *name, Tcl_WideInt *value) {
	size_t i;
	for (i = 0; tcl_constants[i].name; i++) {
		if (!strcmp (tcl_constants[i].name, name)) {
			*value = tcl_constants[i].value;
			return TCL_OK;
		}
	}
	return TCL_ERROR;
}

static void tcl_install_constants(Tcl_Interp *interp) {
	Tcl_CreateNamespace (interp, "::r2", NULL, NULL);
	Tcl_CreateNamespace (interp, "::r2::const", NULL, NULL);
	size_t i;
	for (i = 0; tcl_constants[i].name; i++) {
		Tcl_Obj *value = Tcl_NewWideIntObj (tcl_constants[i].value);
		char *varname = r_str_newf ("::r2::const::%s", tcl_constants[i].name);
		Tcl_IncrRefCount (value);
		Tcl_SetVar2Ex (interp, varname, NULL, value, TCL_GLOBAL_ONLY);
		Tcl_DecrRefCount (value);
		free (varname);
	}
}

static int tcl_get_dict_value(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, Tcl_Obj **out) {
	Tcl_Obj *dictKey = Tcl_NewStringObj (key, -1);
	Tcl_IncrRefCount (dictKey);
	const int rc = Tcl_DictObjGet (interp, dictObj, dictKey, out);
	Tcl_DecrRefCount (dictKey);
	return rc;
}

static Tcl_Obj *tcl_dup_obj(Tcl_Obj *obj) {
	if (!obj) {
		return NULL;
	}
	Tcl_IncrRefCount (obj);
	return obj;
}

static void tcl_obj_unref(Tcl_Obj *obj) {
	if (obj) {
		Tcl_DecrRefCount (obj);
	}
}

static int tcl_eval_prefix(Tcl_Interp *interp, Tcl_Obj *prefix, int argc, Tcl_Obj *const argv[], Tcl_Obj **result) {
	Tcl_Size prefixc = 0;
	Tcl_Obj **prefixv = NULL;
	if (Tcl_ListObjGetElements (interp, prefix, &prefixc, &prefixv) != TCL_OK) {
		return TCL_ERROR;
	}
	const int objc = (int)prefixc + argc;
	Tcl_Obj **objv = calloc (objc, sizeof (Tcl_Obj *));
	if (!objv) {
		Tcl_SetResult (interp, "out of memory", TCL_STATIC);
		return TCL_ERROR;
	}
	int i;
	for (i = 0; i < (int)prefixc; i++) {
		objv[i] = prefixv[i];
	}
	for (i = 0; i < argc; i++) {
		objv[prefixc + i] = argv[i];
	}
	const int rc = Tcl_EvalObjv (interp, objc, objv, TCL_EVAL_GLOBAL);
	free (objv);
	if (rc == TCL_OK && result) {
		*result = Tcl_GetObjResult (interp);
		Tcl_IncrRefCount (*result);
	}
	return rc;
}

static Tcl_Obj *tcl_resolve_provider(Tcl_Interp *interp, Tcl_Obj *provider) {
	Tcl_Obj *value = NULL;
	if (tcl_get_dict_value (interp, provider, "name", &value) == TCL_OK && value) {
		Tcl_IncrRefCount (provider);
		return provider;
	}
	tcl_reset_result (interp);
	Tcl_Obj *result = NULL;
	if (tcl_eval_prefix (interp, provider, 0, NULL, &result) != TCL_OK) {
		return NULL;
	}
	return result;
}

static char *tcl_dict_dup_string(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required) {
	Tcl_Obj *value = NULL;
	if (tcl_get_dict_value (interp, dictObj, key, &value) != TCL_OK) {
		return NULL;
	}
	if (!value) {
		if (required) {
			Tcl_SetObjResult (interp, Tcl_NewStringObj ("missing required field", -1));
		}
		return NULL;
	}
	const char *s = Tcl_GetString (value);
	if (required && R_STR_ISEMPTY (s)) {
		Tcl_SetObjResult (interp, Tcl_NewStringObj ("invalid empty string field", -1));
		return NULL;
	}
	return s? strdup (s): NULL;
}

static Tcl_Obj *tcl_dict_dup_callback(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required) {
	Tcl_Obj *value = NULL;
	if (tcl_get_dict_value (interp, dictObj, key, &value) != TCL_OK) {
		return NULL;
	}
	if (!value) {
		if (required) {
			Tcl_SetObjResult (interp, Tcl_NewStringObj ("missing required callback", -1));
		}
		return NULL;
	}
	Tcl_IncrRefCount (value);
	return value;
}

static bool tcl_obj_to_boolish(Tcl_Interp *interp, Tcl_Obj *obj, bool *out) {
	int b = 0;
	if (Tcl_GetBooleanFromObj (interp, obj, &b) == TCL_OK) {
		*out = b ? true : false;
		return true;
	}
	tcl_reset_result (interp);
	Tcl_WideInt n = 0;
	if (Tcl_GetWideIntFromObj (interp, obj, &n) == TCL_OK) {
		*out = n != 0;
		return true;
	}
	tcl_reset_result (interp);
	const char *s = Tcl_GetString (obj);
	if (R_STR_ISNOTEMPTY (s)) {
		*out = true;
		return true;
	}
	*out = false;
	return true;
}

static bool tcl_obj_to_wide(Tcl_Interp *interp, Tcl_Obj *obj, Tcl_WideInt *out) {
	if (Tcl_GetWideIntFromObj (interp, obj, out) == TCL_OK) {
		return true;
	}
	tcl_reset_result (interp);
	const char *name = Tcl_GetString (obj);
	if (R_STR_ISNOTEMPTY (name) && tcl_named_constant (name, out) == TCL_OK) {
		return true;
	}
	tcl_reset_result (interp);
	return false;
}

static TclPluginContext *tcl_context(RCore *core) {
	return core? R_UNWRAP4 (core, lang, session, plugin_data): NULL;
}

static void tcl_corehack_free(void *p) {
	TclCorePlugin *plugin = (TclCorePlugin *)p;
	if (!plugin) {
		return;
	}
	tcl_obj_unref (plugin->call);
	free (plugin->name);
	free (plugin);
}

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

static int r2_cmd_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 2) {
		Tcl_WrongNumArgs (interp, 1, objv, "command");
		return TCL_ERROR;
	}
	const char *cmd = Tcl_GetString (objv[1]);
	char *res = r_core_cmd_str (core, cmd);
	Tcl_SetResult (interp, res? res: strdup (""), (Tcl_FreeProc *)tcl_free);
	return TCL_OK;
}

static int r2_print_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 2) {
		Tcl_WrongNumArgs (interp, 1, objv, "text");
		return TCL_ERROR;
	}
	r_cons_print (core->cons, Tcl_GetString (objv[1]));
	r_cons_print (core->cons, "\n");
	return TCL_OK;
}

static int r2_flush_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 1) {
		Tcl_WrongNumArgs (interp, 1, objv, "");
		return TCL_ERROR;
	}
	r_cons_flush (core->cons);
	return TCL_OK;
}

static int r2_const_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc == 1) {
		Tcl_Obj *list = Tcl_NewListObj (0, NULL);
		size_t i;
		for (i = 0; tcl_constants[i].name; i++) {
			Tcl_ListObjAppendElement (interp, list, Tcl_NewStringObj (tcl_constants[i].name, -1));
		}
		Tcl_SetObjResult (interp, list);
		return TCL_OK;
	}
	if (objc == 2) {
		Tcl_WideInt value = 0;
		if (tcl_named_constant (Tcl_GetString (objv[1]), &value) == TCL_OK) {
			Tcl_SetObjResult (interp, Tcl_NewWideIntObj (value));
			return TCL_OK;
		}
		Tcl_SetResult (interp, "unknown constant", TCL_STATIC);
		return TCL_ERROR;
	}
	Tcl_WrongNumArgs (interp, 1, objv, "?name?");
	return TCL_ERROR;
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
		handled = b ? true : false;
		tcl_obj_unref (result);
		tcl_reset_result (plugin->interp);
		return handled;
	} else {
		Tcl_WideInt n = 0;
		tcl_reset_result (plugin->interp);
		if (Tcl_GetWideIntFromObj (plugin->interp, result, &n) == TCL_OK) {
			handled = n != 0;
			tcl_obj_unref (result);
			tcl_reset_result (plugin->interp);
			return handled;
		}
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
	if (!strcmp (value, "al")) return R_ANAL_CONDTYPE_AL;
	if (!strcmp (value, "eq")) return R_ANAL_CONDTYPE_EQ;
	if (!strcmp (value, "ne")) return R_ANAL_CONDTYPE_NE;
	if (!strcmp (value, "ge")) return R_ANAL_CONDTYPE_GE;
	if (!strcmp (value, "gt")) return R_ANAL_CONDTYPE_GT;
	if (!strcmp (value, "le")) return R_ANAL_CONDTYPE_LE;
	if (!strcmp (value, "lt")) return R_ANAL_CONDTYPE_LT;
	if (!strcmp (value, "nv")) return R_ANAL_CONDTYPE_NV;
	if (!strcmp (value, "hs")) return R_ANAL_CONDTYPE_HS;
	if (!strcmp (value, "lo")) return R_ANAL_CONDTYPE_LO;
	if (!strcmp (value, "mi")) return R_ANAL_CONDTYPE_MI;
	if (!strcmp (value, "pl")) return R_ANAL_CONDTYPE_PL;
	if (!strcmp (value, "vs")) return R_ANAL_CONDTYPE_VS;
	if (!strcmp (value, "vc")) return R_ANAL_CONDTYPE_VC;
	if (!strcmp (value, "hi")) return R_ANAL_CONDTYPE_HI;
	if (!strcmp (value, "ls")) return R_ANAL_CONDTYPE_LS;
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
	if (!strcmp (key, "mnemonic") || !strcmp (key, "esil") || !strcmp (key, "opex")
	 || !strcmp (key, "reg") || !strcmp (key, "ireg")) {
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

static int r2_plugin_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	TclPluginContext *ctx = (TclPluginContext *)clientData;
	if (!ctx || !ctx->core) {
		return TCL_ERROR;
	}
	if (objc != 3) {
		Tcl_WrongNumArgs (interp, 1, objv, "type provider");
		return TCL_ERROR;
	}
	const char *type = Tcl_GetString (objv[1]);
	Tcl_Obj *spec = tcl_resolve_provider (interp, objv[2]);
	if (!spec) {
		return TCL_ERROR;
	}
	int ret = 0;
	if (!strcmp (type, "core")) {
		char *name = tcl_dict_dup_string (interp, spec, "name", true);
		Tcl_Obj *call = tcl_dict_dup_callback (interp, spec, "call", true);
		char *desc = tcl_dict_dup_string (interp, spec, "desc", false);
		char *license = tcl_dict_dup_string (interp, spec, "license", false);
		if (name && call && !tcl_find_core_plugin (ctx, name)) {
			RCorePlugin *plugin = R_NEW0 (RCorePlugin);
			TclCorePlugin *hack = R_NEW0 (TclCorePlugin);
			if (plugin && hack) {
				plugin->meta.name = strdup (name);
				plugin->meta.desc = desc? strdup (desc): NULL;
				plugin->meta.license = license? strdup (license): NULL;
				plugin->init = tcl_core_init;
				plugin->fini = tcl_core_fini;
				plugin->call = tcl_core_call;
				hack->name = name;
				hack->interp = interp;
				hack->call = call;
				if (!ctx->core_plugins) {
					ctx->core_plugins = r_list_newf (tcl_corehack_free);
				}
				r_list_append (ctx->core_plugins, hack);
				RLibStruct lib = {
					.type = R_LIB_TYPE_CORE,
					.data = plugin,
					.free = (void (*)(void *))tcl_core_plugin_free,
					.version = R2_VERSION,
				};
				ret = r_lib_open_ptr (ctx->core->lib, plugin->meta.name, NULL, &lib);
				if (ret == 1) {
					hack = NULL;
				} else {
					r_list_delete_data (ctx->core_plugins, hack);
				}
			}
			if (hack) {
				tcl_corehack_free (hack);
			}
			if (!ret && plugin) {
				tcl_core_plugin_free (plugin);
			}
		}
		if (!ret) {
			tcl_obj_unref (call);
			free (name);
		}
		free (desc);
		free (license);
	} else if (!strcmp (type, "io")) {
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
			if (plugin && hack) {
				RPluginMeta meta = {
					.name = strdup (name),
					.desc = desc? strdup (desc): NULL,
					.license = license? strdup (license): NULL,
				};
				memcpy ((void *)&plugin->meta, &meta, sizeof (meta));
				plugin->data = hack;
				plugin->uris = uris? strdup (uris): NULL;
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
				hack->uris = uris? strdup (uris): NULL;
				if (!ctx->io_plugins) {
					ctx->io_plugins = r_list_newf (tcl_iohack_free);
				}
				RLibStruct lib = {
					.type = R_LIB_TYPE_IO,
					.data = plugin,
					.free = (void (*)(void *))tcl_io_plugin_free,
					.version = R2_VERSION,
				};
				ret = r_lib_open_ptr (ctx->core->lib, plugin->meta.name, NULL, &lib);
				if (ret == 1) {
					r_list_append (ctx->io_plugins, hack);
					hack = NULL;
				}
			}
			if (hack) {
				tcl_iohack_free (hack);
			}
			if (!ret && plugin) {
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
	} else if (!strcmp (type, "arch")) {
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
		Tcl_Obj *bitsObj = NULL;
		Tcl_Obj *endianObj = NULL;
		Tcl_Obj *addrBitsObj = NULL;
		ut32 bits = 0;
		ut32 endian = 0;
		ut32 addr_bits = 0;
		if (tcl_get_dict_value (interp, spec, "bits", &bitsObj) == TCL_OK && bitsObj) {
			Tcl_WideInt n = 0;
			if (tcl_obj_to_wide (interp, bitsObj, &n)) {
				bits = (ut32)n;
			}
		}
		tcl_reset_result (interp);
		if (tcl_get_dict_value (interp, spec, "endian", &endianObj) == TCL_OK && endianObj) {
			const char *s = Tcl_GetString (endianObj);
			if (!strcmp (s, "little")) {
				endian = R_SYS_ENDIAN_LITTLE;
			} else if (!strcmp (s, "big")) {
				endian = R_SYS_ENDIAN_BIG;
			} else {
				Tcl_WideInt n = 0;
				if (tcl_obj_to_wide (interp, endianObj, &n)) {
					endian = (ut32)n;
				}
			}
		}
		tcl_reset_result (interp);
		if (tcl_get_dict_value (interp, spec, "addr_bits", &addrBitsObj) == TCL_OK && addrBitsObj) {
			Tcl_WideInt n = 0;
			if (tcl_obj_to_wide (interp, addrBitsObj, &n)) {
				addr_bits = (ut32)n;
			}
		}
		tcl_reset_result (interp);
		if (name && arch && decode && bits && !tcl_find_arch_plugin_by_name (ctx, name) && !tcl_find_arch_plugin_by_arch (ctx, arch)) {
			RArchPlugin *plugin = R_NEW0 (RArchPlugin);
			TclArchPlugin *hack = R_NEW0 (TclArchPlugin);
			if (plugin && hack) {
				plugin->meta.name = strdup (name);
				plugin->meta.desc = desc? strdup (desc): NULL;
				plugin->meta.author = author? strdup (author): NULL;
				plugin->meta.license = license? strdup (license): NULL;
				plugin->meta.version = version? strdup (version): NULL;
				plugin->arch = strdup (arch);
				plugin->cpus = cpus? strdup (cpus): NULL;
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
				if (!ctx->arch_plugins) {
					ctx->arch_plugins = r_list_newf (tcl_archhack_free);
				}
				RLibStruct lib = {
					.type = R_LIB_TYPE_ARCH,
					.data = plugin,
					.free = (void (*)(void *))tcl_arch_plugin_free,
					.version = R2_VERSION,
				};
				ret = r_lib_open_ptr (ctx->core->lib, plugin->meta.name, NULL, &lib);
				if (ret == 1) {
					r_list_append (ctx->arch_plugins, hack);
					hack = NULL;
				}
			}
			if (hack) {
				tcl_archhack_free (hack);
			}
			if (!ret && plugin) {
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
	} else {
		tcl_obj_unref (spec);
		Tcl_SetResult (interp, "unsupported plugin type", TCL_STATIC);
		return TCL_ERROR;
	}
	tcl_obj_unref (spec);
	Tcl_SetObjResult (interp, Tcl_NewBooleanObj (ret == 1));
	return TCL_OK;
}

static bool runstr(RLangSession *s, const char *code, int len) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_Eval (interp, code) == TCL_ERROR) {
		R_LOG_ERROR ("TCL Error: %s", Tcl_GetStringResult (interp));
		tcl_reset_result (interp);
		return false;
	}
	tcl_reset_result (interp);
	return true;
}

static bool init(RLangSession * R_NULLABLE s) {
	if (s == NULL) {
		return true;
	}
	TclPluginContext *pluginContext = R_NEW0 (TclPluginContext);
	if (!pluginContext) {
		return false;
	}
	pluginContext->core = s->lang->user;
	pluginContext->interp = Tcl_CreateInterp ();
	if (!pluginContext->interp) {
		free (pluginContext);
		return false;
	}
	if (Tcl_Init (pluginContext->interp) != TCL_OK) {
		R_LOG_ERROR ("TCL init error: %s", Tcl_GetStringResult (pluginContext->interp));
		Tcl_DeleteInterp (pluginContext->interp);
		free (pluginContext);
		return false;
	}
	tcl_install_constants (pluginContext->interp);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::cmd", r2_cmd_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::print", r2_print_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::flush", r2_flush_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::const", r2_const_objcmd, pluginContext->core, NULL);
	Tcl_CreateObjCommand (pluginContext->interp, "::r2::plugin", r2_plugin_objcmd, pluginContext, NULL);
	Tcl_Eval (pluginContext->interp,
		"namespace eval ::r2 { namespace export cmd print flush const plugin; namespace ensemble create }\n"
		"interp alias {} r2cmd {} ::r2::cmd\n"
		"interp alias {} r2plugin {} ::r2::plugin\n"
		"interp alias {} r2print {} ::r2::print\n"
		"interp alias {} r2flush {} ::r2::flush\n"
		"interp alias {} r2const {} ::r2::const\n");
	s->plugin_data = pluginContext;
	return true;
}

static bool fini(RLangSession *s) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	if (!pluginContext) {
		return true;
	}
	if (pluginContext->core_plugins) {
		r_list_free (pluginContext->core_plugins);
	}
	if (pluginContext->io_plugins) {
		r_list_free (pluginContext->io_plugins);
	}
	if (pluginContext->arch_plugins) {
		r_list_free (pluginContext->arch_plugins);
	}
	if (pluginContext->interp) {
		Tcl_DeleteInterp (pluginContext->interp);
	}
	R_FREE (s->plugin_data);
	return true;
}

static bool runfile(RLangSession *s, const char *file) {
	TclPluginContext *pluginContext = (TclPluginContext *)s->plugin_data;
	Tcl_Interp *interp = pluginContext->interp;
	if (Tcl_EvalFile (interp, file) == TCL_ERROR) {
		R_LOG_ERROR ("TCL Error: %s", Tcl_GetStringResult (interp));
		tcl_reset_result (interp);
		return false;
	}
	tcl_reset_result (interp);
	return true;
}

static RLangPlugin r_lang_plugin_tcl = {
	.meta = {
		.name = "tcl",
		.license = "MIT",
		.desc = "TCL/TK scripting for radare2",
		.author = "pancake"
	},
	.ext = "tcl",
	.init = (void*)init,
	.fini = (void*)fini,
	.run = runstr,
	.run_file = (void*)runfile,
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_LANG,
	.data = &r_lang_plugin_tcl,
};
#endif
