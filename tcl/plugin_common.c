/* lang.tcl plugin for r2 - 2023-2026 - pancake */

#include "plugin.h"

#define TCL_CONSTANT(name) { #name, (Tcl_WideInt) (name) }

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
	{ NULL, 0 }
};

char *tcl_strdup0(const char *s) {
	return s? strdup (s): NULL;
}

void tcl_reset_result(Tcl_Interp *interp) {
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

void tcl_install_constants(Tcl_Interp *interp) {
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

int tcl_get_dict_value(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, Tcl_Obj **out) {
	Tcl_Obj *dictKey = Tcl_NewStringObj (key, -1);
	Tcl_IncrRefCount (dictKey);
	const int rc = Tcl_DictObjGet (interp, dictObj, dictKey, out);
	Tcl_DecrRefCount (dictKey);
	return rc;
}

Tcl_Obj *tcl_dup_obj(Tcl_Obj *obj) {
	if (!obj) {
		return NULL;
	}
	Tcl_IncrRefCount (obj);
	return obj;
}

void tcl_obj_unref(Tcl_Obj *obj) {
	if (obj) {
		Tcl_DecrRefCount (obj);
	}
}

int tcl_eval_prefix(Tcl_Interp *interp, Tcl_Obj *prefix, int argc, Tcl_Obj *const argv[], Tcl_Obj **result) {
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

Tcl_Obj *tcl_resolve_provider(Tcl_Interp *interp, Tcl_Obj *provider) {
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

char *tcl_dict_dup_string(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required) {
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

Tcl_Obj *tcl_dict_dup_callback(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required) {
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

bool tcl_obj_to_boolish(Tcl_Interp *interp, Tcl_Obj *obj, bool *out) {
	int b = 0;
	if (Tcl_GetBooleanFromObj (interp, obj, &b) == TCL_OK) {
		*out = b != 0;
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

bool tcl_obj_to_wide(Tcl_Interp *interp, Tcl_Obj *obj, Tcl_WideInt *out) {
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

ut32 tcl_dict_get_u32(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key) {
	Tcl_Obj *valueObj = NULL;
	Tcl_WideInt n = 0;
	ut32 value = 0;
	if (tcl_get_dict_value (interp, dictObj, key, &valueObj) == TCL_OK && valueObj && tcl_obj_to_wide (interp, valueObj, &n)) {
		value = (ut32)n;
	}
	tcl_reset_result (interp);
	return value;
}

ut32 tcl_dict_get_endian(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key) {
	Tcl_Obj *valueObj = NULL;
	ut32 value = 0;
	if (tcl_get_dict_value (interp, dictObj, key, &valueObj) == TCL_OK && valueObj) {
		const char *s = Tcl_GetString (valueObj);
		if (!strcmp (s, "little")) {
			value = R_SYS_ENDIAN_LITTLE;
		} else if (!strcmp (s, "big")) {
			value = R_SYS_ENDIAN_BIG;
		} else {
			Tcl_WideInt n = 0;
			if (tcl_obj_to_wide (interp, valueObj, &n)) {
				value = (ut32)n;
			}
		}
	}
	tcl_reset_result (interp);
	return value;
}

TclPluginContext *tcl_context(RCore *core) {
	return core? R_UNWRAP4 (core, lang, session, plugin_data): NULL;
}

RList *tcl_ensure_plugin_list(RList **list, RListFree freefn) {
	if (!list) {
		return NULL;
	}
	if (!*list) {
		*list = r_list_newf (freefn);
	}
	return *list;
}

int tcl_open_plugin(TclPluginContext *ctx, const char *name, int type, void *data, void(*freefn)(void *)) {
	RLibStruct lib = { .type = type, .data = data, .free = freefn, .version = R2_VERSION };
	return r_lib_open_ptr (ctx->core->lib, name, NULL, &lib);
}

int r2_cmd_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 2) {
		Tcl_WrongNumArgs (interp, 1, objv, "command");
		return TCL_ERROR;
	}
	const char *cmd = Tcl_GetString (objv[1]);
	char *res = r_core_cmd_str (core, cmd);
	Tcl_SetResult (interp, res? res: strdup (""), (Tcl_FreeProc *)free);
	return TCL_OK;
}

int r2_print_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 2) {
		Tcl_WrongNumArgs (interp, 1, objv, "text");
		return TCL_ERROR;
	}
	r_cons_print (core->cons, Tcl_GetString (objv[1]));
	r_cons_print (core->cons, "\n");
	return TCL_OK;
}

int r2_flush_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	RCore *core = (RCore *)clientData;
	if (objc != 1) {
		Tcl_WrongNumArgs (interp, 1, objv, "");
		return TCL_ERROR;
	}
	r_cons_flush (core->cons);
	return TCL_OK;
}

int r2_const_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
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
