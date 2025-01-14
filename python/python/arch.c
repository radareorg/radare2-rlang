/* radare - LGPL - Copyright 2024-2025 - astuder */

// Exporting the R_ARCH_* enum constants
#include <r_reg.h>
#include "core.h"

#if R2_VERSION_NUMBER > 50808

#include "arch.h"

static void *py_arch_init_cb = NULL;
static void *py_arch_fini_cb = NULL;
static void *py_arch_info_cb = NULL;
static void *py_arch_regs_cb = NULL;
static void *py_arch_encode_cb = NULL;
static void *py_arch_decode_cb = NULL;
static void *py_arch_patch_cb = NULL;
static void *py_arch_mnemonics_cb = NULL;
static void *py_arch_preludes_cb = NULL;
static void *py_arch_esilcb_cb = NULL;

void py_export_arch_enum(PyObject *tp_dict) {

#define PYENUM(name) {\
		PyObject *o = PyLong_FromLong(name); \
		if (o) { \
			PyDict_SetItemString(tp_dict, #name, o); \
			Py_DECREF(o); \
		}\
	}

	// arch info, see radare2/libr/include/r_arch.h for documentation
	PYENUM (R_ARCH_INFO_MINOP_SIZE);
	PYENUM (R_ARCH_INFO_MAXOP_SIZE);
	PYENUM (R_ARCH_INFO_INVOP_SIZE);
	PYENUM (R_ARCH_INFO_CODE_ALIGN);
	PYENUM (R_ARCH_INFO_DATA_ALIGN);
	PYENUM (R_ARCH_INFO_DATA2_ALIGN);
	PYENUM (R_ARCH_INFO_DATA4_ALIGN);
	PYENUM (R_ARCH_INFO_DATA8_ALIGN);

	// decode/encode operations, see radare2/libr/include/r_arch.h for documentation
	PYENUM (R_ARCH_OP_MASK_BASIC);
	PYENUM (R_ARCH_OP_MASK_ESIL);
	PYENUM (R_ARCH_OP_MASK_VAL);
	PYENUM (R_ARCH_OP_MASK_HINT);
	PYENUM (R_ARCH_OP_MASK_OPEX);
	PYENUM (R_ARCH_OP_MASK_DISASM);
	PYENUM (R_ARCH_OP_MASK_ALL);

	// opcode type, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ARCH_OP_MOD_COND);
	PYENUM (R_ARCH_OP_MOD_REP);
	PYENUM (R_ARCH_OP_MOD_MEM);
	PYENUM (R_ARCH_OP_MOD_REG);
	PYENUM (R_ARCH_OP_MOD_IND);
	PYENUM (R_ANAL_OP_TYPE_NULL);
	PYENUM (R_ANAL_OP_TYPE_JMP);
	PYENUM (R_ANAL_OP_TYPE_UJMP);
	PYENUM (R_ANAL_OP_TYPE_RJMP);
	PYENUM (R_ANAL_OP_TYPE_UCJMP);
	PYENUM (R_ANAL_OP_TYPE_IJMP);
	PYENUM (R_ANAL_OP_TYPE_IRJMP);
	PYENUM (R_ANAL_OP_TYPE_CJMP);
	PYENUM (R_ANAL_OP_TYPE_MJMP);
	PYENUM (R_ANAL_OP_TYPE_RCJMP);
	PYENUM (R_ANAL_OP_TYPE_MCJMP);
	PYENUM (R_ANAL_OP_TYPE_CALL);
	PYENUM (R_ANAL_OP_TYPE_UCALL);
	PYENUM (R_ANAL_OP_TYPE_RCALL);
	PYENUM (R_ANAL_OP_TYPE_ICALL);
	PYENUM (R_ANAL_OP_TYPE_IRCALL);
	PYENUM (R_ANAL_OP_TYPE_CCALL);
	PYENUM (R_ANAL_OP_TYPE_UCCALL);
	PYENUM (R_ANAL_OP_TYPE_RET);
	PYENUM (R_ANAL_OP_TYPE_CRET);
	PYENUM (R_ANAL_OP_TYPE_ILL);
	PYENUM (R_ANAL_OP_TYPE_UNK);
	PYENUM (R_ANAL_OP_TYPE_NOP);
	PYENUM (R_ANAL_OP_TYPE_MOV);
	PYENUM (R_ANAL_OP_TYPE_CMOV);
	PYENUM (R_ANAL_OP_TYPE_TRAP);
	PYENUM (R_ANAL_OP_TYPE_SWI);
	PYENUM (R_ANAL_OP_TYPE_CSWI);
	PYENUM (R_ANAL_OP_TYPE_UPUSH);
	PYENUM (R_ANAL_OP_TYPE_RPUSH);
	PYENUM (R_ANAL_OP_TYPE_PUSH);
	PYENUM (R_ANAL_OP_TYPE_POP);
	PYENUM (R_ANAL_OP_TYPE_CMP);
	PYENUM (R_ANAL_OP_TYPE_ACMP);
	PYENUM (R_ANAL_OP_TYPE_ADD);
	PYENUM (R_ANAL_OP_TYPE_SUB);
	PYENUM (R_ANAL_OP_TYPE_IO);
	PYENUM (R_ANAL_OP_TYPE_MUL);
	PYENUM (R_ANAL_OP_TYPE_DIV);
	PYENUM (R_ANAL_OP_TYPE_SHR);
	PYENUM (R_ANAL_OP_TYPE_SHL);
	PYENUM (R_ANAL_OP_TYPE_SAL);
	PYENUM (R_ANAL_OP_TYPE_SAR);
	PYENUM (R_ANAL_OP_TYPE_OR);
	PYENUM (R_ANAL_OP_TYPE_AND);
	PYENUM (R_ANAL_OP_TYPE_XOR);
	PYENUM (R_ANAL_OP_TYPE_NOR);
	PYENUM (R_ANAL_OP_TYPE_NOT);
	PYENUM (R_ANAL_OP_TYPE_STORE);
	PYENUM (R_ANAL_OP_TYPE_LOAD);
	PYENUM (R_ANAL_OP_TYPE_LEA);
	PYENUM (R_ANAL_OP_TYPE_ULEA);
	PYENUM (R_ANAL_OP_TYPE_LEAVE);
	PYENUM (R_ANAL_OP_TYPE_ROR);
	PYENUM (R_ANAL_OP_TYPE_ROL);
	PYENUM (R_ANAL_OP_TYPE_XCHG);
	PYENUM (R_ANAL_OP_TYPE_MOD);
	PYENUM (R_ANAL_OP_TYPE_SWITCH);
	PYENUM (R_ANAL_OP_TYPE_CASE);
	PYENUM (R_ANAL_OP_TYPE_LENGTH);
	PYENUM (R_ANAL_OP_TYPE_CAST);
	PYENUM (R_ANAL_OP_TYPE_NEW);
	PYENUM (R_ANAL_OP_TYPE_ABS);
	PYENUM (R_ANAL_OP_TYPE_CPL);
	PYENUM (R_ANAL_OP_TYPE_CRYPTO);
	PYENUM (R_ANAL_OP_TYPE_SYNC);
	PYENUM (R_ANAL_OP_TYPE_DEBUG);

	// opcode prefix, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ANAL_OP_PREFIX_COND);
	PYENUM (R_ANAL_OP_PREFIX_REP);
	PYENUM (R_ANAL_OP_PREFIX_REPNE);
	PYENUM (R_ANAL_OP_PREFIX_LOCK);
	PYENUM (R_ANAL_OP_PREFIX_LIKELY);
	PYENUM (R_ANAL_OP_PREFIX_UNLIKELY);

	// opcode stack operation, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ANAL_STACK_NULL);
	PYENUM (R_ANAL_STACK_NOP);
	PYENUM (R_ANAL_STACK_INC);
	PYENUM (R_ANAL_STACK_GET);
	PYENUM (R_ANAL_STACK_SET);
	PYENUM (R_ANAL_STACK_RESET);
	PYENUM (R_ANAL_STACK_ALIGN);
#if 0
	// opcode condition, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ANAL_COND_AL);
	PYENUM (R_ANAL_COND_EQ);
	PYENUM (R_ANAL_COND_NE);
	PYENUM (R_ANAL_COND_GE);
	PYENUM (R_ANAL_COND_GT);
	PYENUM (R_ANAL_COND_LE);
	PYENUM (R_ANAL_COND_LT);
	PYENUM (R_ANAL_COND_NV);
	PYENUM (R_ANAL_COND_HS);
	PYENUM (R_ANAL_COND_LO);
	PYENUM (R_ANAL_COND_MI);
	PYENUM (R_ANAL_COND_PL);
	PYENUM (R_ANAL_COND_VS);
	PYENUM (R_ANAL_COND_VC);
	PYENUM (R_ANAL_COND_HI);
	PYENUM (R_ANAL_COND_LS);
#endif
	// opcode direction, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ANAL_OP_DIR_READ);
	PYENUM (R_ANAL_OP_DIR_WRITE);
	PYENUM (R_ANAL_OP_DIR_EXEC);
	PYENUM (R_ANAL_OP_DIR_REF);

	// opcode family, see radare2/libr/include/r_anal/op.h for documentation	
	PYENUM (R_ANAL_OP_FAMILY_UNKNOWN);
	PYENUM (R_ANAL_OP_FAMILY_CPU);
	PYENUM (R_ANAL_OP_FAMILY_FPU);
	PYENUM (R_ANAL_OP_FAMILY_VEC);
	PYENUM (R_ANAL_OP_FAMILY_PRIV);
	PYENUM (R_ANAL_OP_FAMILY_CRYPTO);
	PYENUM (R_ANAL_OP_FAMILY_THREAD);
	PYENUM (R_ANAL_OP_FAMILY_VIRT);
	PYENUM (R_ANAL_OP_FAMILY_SECURITY);
	PYENUM (R_ANAL_OP_FAMILY_IO);
	PYENUM (R_ANAL_OP_FAMILY_SIMD);

	// opcode family, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM (R_ANAL_DATATYPE_NULL);
	PYENUM (R_ANAL_DATATYPE_ARRAY);
	PYENUM (R_ANAL_DATATYPE_OBJECT);
	PYENUM (R_ANAL_DATATYPE_STRING);
	PYENUM (R_ANAL_DATATYPE_CLASS);
	PYENUM (R_ANAL_DATATYPE_BOOLEAN);
	PYENUM (R_ANAL_DATATYPE_INT16);
	PYENUM (R_ANAL_DATATYPE_INT32);
	PYENUM (R_ANAL_DATATYPE_INT64);
	PYENUM (R_ANAL_DATATYPE_FLOAT);

#undef PYENUM
}

static bool py_arch_init(RArchSession *as) {
//	R_LOG_INFO ("py_arch_init called");
	r_return_val_if_fail (as, false);
	int res = true;	// it's ok if init is missing
	if (py_arch_init_cb) {
		PyObject *result = PyObject_CallObject (py_arch_init_cb, NULL);
		if (result) {
			// no return value expected
			Py_DECREF (result);
		}
		if (PyErr_Occurred ()) {
			PyErr_Print ();
			res = false;
		}
	}
	return res;
}

static bool py_arch_fini(RArchSession *as) {
	// is this needed/called?
	R_LOG_WARN ("py_arch_fini not implemented");
	r_return_val_if_fail (as, false);
	return true;
}

static int py_arch_info(RArchSession *as, ut32 q) {
//	R_LOG_INFO ("py_arch_info called");	
	r_return_val_if_fail (as, -1);
	int res = -1;
	if (py_arch_info_cb) {
		PyObject *arglist = Py_BuildValue ("(i)", q);
		PyObject *result = PyObject_CallObject (py_arch_info_cb, arglist);
		if (result) {
			if (PyLong_Check (result)) {
				res = PyLong_AsLong (result);
			}
			Py_DECREF (result);
		}
		Py_DECREF (arglist);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
		}
	}
	return res;
}

static char *py_arch_regs(RArchSession *as) {
//	R_LOG_INFO ("py_arch_regs called");
	r_return_val_if_fail (as, NULL);
	char *res = NULL;
	if (py_arch_regs_cb) {
		PyObject *result = PyObject_CallObject (py_arch_regs_cb, NULL);
		if (result) {
			if (PyUnicode_Check (result)) {
				const char* regs = PyUnicode_AsUTF8 (result);
				if (regs) {
					res = strdup(regs);
				}
			}
			Py_DECREF (result);
		}
		if (PyErr_Occurred ()) {
			PyErr_Print ();
		}
	}
	return res;
}

static bool py_arch_encode(RArchSession *as, RAnalOp *op, RArchEncodeMask mask) {
//	R_LOG_INFO ("py_arch_encode called");
	r_return_val_if_fail (as && op, false);
	bool res = false;
	if (py_arch_encode_cb) {
		PyObject *arglist = Py_BuildValue ("(Ks)", op->addr, op->mnemonic);
		PyObject *result = PyObject_CallObject (py_arch_encode_cb, arglist);
		if (result) {
			if (PyBytes_Check (result)) {
				int size = PyBytes_Size (result);
				if (size > 0) {
					free (op->bytes);
					op->bytes = r_mem_dup (PyBytes_AsString (result), size);
					op->size = size;
					res = true;
				} else {
					R_LOG_WARN ("No assembled bytes returned by Python arch plugin");
				}
			}
			Py_DECREF (result);
		}
		Py_DECREF (arglist);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
		}
	} else {
		R_LOG_WARN ("Python arch plugin does not implement encode");
	}
	return res;
}

static bool py_arch_decode(RArchSession *as, RAnalOp *op, RAnalOpMask mask) {
//	R_LOG_INFO ("py_arch_decode called");
	r_return_val_if_fail (as && op, false);
	bool res = false;
	if (py_arch_decode_cb) {
		Py_buffer pybuf = {
			.buf = (void *) op->bytes,
			.len = op->size,
			.readonly = 1,
			.ndim = 1,
			.itemsize = 1
		};
		PyObject *memview = PyMemoryView_FromBuffer (&pybuf);
		PyObject *arglist = Py_BuildValue ("(NKi)", memview, op->addr, mask);
		PyObject *result = PyObject_CallObject (py_arch_decode_cb, arglist);
		if (result) {
			if (PyList_Check (result)) {
				PyObject *len = PyList_GetItem (result, 0);
				PyObject *dict = PyList_GetItem (result, 1);
				if (len && dict) {
					op->size = PyNumber_AsSsize_t (len, NULL);
					PyObject *key, *value;
					Py_ssize_t pos = 0;
					while (PyDict_Next (dict, &pos, &key, &value)) {
						const char* key_str = PyUnicode_AsUTF8 (key);
						int i = 0;
						if (value == Py_None) {
							R_LOG_WARN("Python arch plugin: field '%s' is None", key_str);
						} else if (strcmp (key_str, "mnemonic") == 0) {
							op->mnemonic = strdup (PyUnicode_AsUTF8 (value));
						} else if (strcmp (key_str, "type") == 0) {
							op->type = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "prefix") == 0) {
							op->prefix = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "type2") == 0) {
							op->type2 = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "stackop") == 0) {
							op->stackop = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "cond") == 0) {
							op->cond = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "nopcode") == 0) {
							op->nopcode = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "cycles") == 0) {
							op->cycles = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "failcycles") == 0) {
							op->failcycles = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "family") == 0) {
							op->family = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "id") == 0) {
							op->id = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "eob") == 0) {
							op->eob = PyObject_IsTrue (value);
						} else if (strcmp (key_str, "sign") == 0) {
							op->sign = PyObject_IsTrue (value);
						} else if (strcmp (key_str, "delay") == 0) {
							op->delay = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "jump") == 0) {
							op->jump = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "fail") == 0) {
							op->fail = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "direction") == 0) {
							op->direction = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "ptr") == 0) {
							op->ptr = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "val") == 0) {
							op->val = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "ptrsize") == 0) {
							op->ptrsize = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "stackptr") == 0) {
							op->stackptr = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "refptr") == 0) {
							op->refptr = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "esil") == 0) {
							r_strbuf_init(&op->esil);
							r_strbuf_set (&op->esil, PyUnicode_AsUTF8 (value));
						} else if (strcmp (key_str, "opex") == 0) {
							r_strbuf_init(&op->opex);
							r_strbuf_set (&op->opex, PyUnicode_AsUTF8 (value));
						} else if (strcmp (key_str, "reg") == 0) {
							op->reg = strdup (PyUnicode_AsUTF8 (value));
						} else if (strcmp (key_str, "ireg") == 0) {
							op->ireg = strdup (PyUnicode_AsUTF8 (value));
						} else if (strcmp (key_str, "scale") == 0) {
							op->scale = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "disp") == 0) {
							op->disp = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "datatype") == 0) {
							op->datatype = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "vliw") == 0) {
							op->vliw = PyNumber_AsSsize_t (value, NULL);
						} else if (strcmp (key_str, "payload") == 0) {
							op->payload = PyNumber_AsSsize_t (value, NULL);
						} else {
							R_LOG_WARN("Python arch plugin: unknown field '%s'", key_str);
						}
						if (PyErr_Occurred ()) {
							R_LOG_WARN("Pyton arch plugin: field '%s' has wrong data type", key_str);
							PyErr_Print ();
						}
					}
					res = true;
				}
			}
			Py_DECREF (result);
		} 
		Py_DECREF (arglist);
		Py_DECREF (memview);
		if (PyErr_Occurred ()) {
			PyErr_Print ();
		}
	} else {
		R_LOG_WARN ("Python arch plugin does not implement decode");
	}
	return res;
}

static bool py_arch_patch(RArchSession *as, RAnalOp *op, RArchModifyMask mask) {
	r_return_val_if_fail (as && op, false);
	// TODO?
//	R_LOG_WARN ("py_arch_patch not implemented");
	return true;
}

static char *py_arch_mnemonics(RArchSession *as, int id, bool json) {
	r_return_val_if_fail (as, NULL);
	// TODO?
//	R_LOG_WARN ("py_arch_mnemonics not implemented");
	return NULL;
}

static RList *py_arch_preludes(RArchSession *as) {
	r_return_val_if_fail (as, NULL);
	// TODO?
//	R_LOG_WARN ("py_arch_preludes not implemented");
	return NULL;
}

static bool py_arch_esilcb(RArchSession *as, RArchEsilAction action) {
	r_return_val_if_fail (as, false);
	// TODO?
//	R_LOG_WARN ("py_arch_esilcb not implemented");
	return true;
}

void Radare_plugin_arch_free(RArchPlugin *ap) {
	if (ap) {
		free ((char *)ap->meta.name);
		free ((char *)ap->meta.license);
		free ((char *)ap->meta.desc);
		free ((char*)ap->arch);
		free ((char*)ap->cpus);
		free (ap);
	}
}

RArchPlugin py_arch_plugin = {
	.init = py_arch_init,
	.fini = py_arch_fini,
	.info = py_arch_info,
	.regs = py_arch_regs,
	.encode = py_arch_encode,
	.decode = py_arch_decode,
	.patch = py_arch_patch,
	.mnemonics = py_arch_mnemonics,
	.preludes = py_arch_preludes,
	.esilcb = py_arch_esilcb
};

PyObject *Radare_plugin_arch(Radare* self, PyObject *args) {
	PyObject *arglist = Py_BuildValue ("(i)", 0);
	PyObject *o = PyObject_CallObject (args, arglist);
	if (!o) {
		return NULL;
	}

	RArchPlugin *ap = &py_arch_plugin;
	if (!ap) {
		return NULL;
	}
	RPluginMeta meta = {
		.name = getS (o, "name"),
		.license = getS (o, "license"),
		.desc = getS (o, "desc")
	};
	memcpy ((RPluginMeta *)&ap->meta, &meta, sizeof (RPluginMeta));

    ap->arch = getS (o, "arch");
	ap->cpus = getS (o, "cpus");
	ap->endian = getI (o, "endian");
	ap->bits = getI (o, "bits");
	ap->addr_bits = getI (o, "addr_bits");

	void *ptr;
	ptr = getF (o, "init");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_init_cb = ptr;
	}
	ptr = getF (o, "fini");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_fini_cb = ptr;
	}
	ptr = getF (o, "info");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_info_cb = ptr;
	}
	ptr = getF (o, "regs");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_regs_cb = ptr;
	}
	ptr = getF (o, "encode");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_encode_cb = ptr;
	}
	ptr = getF (o, "decode");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_decode_cb = ptr;
	}
	ptr = getF (o, "patch");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_patch_cb = ptr;
	}
	ptr = getF (o, "mnemonics");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_mnemonics_cb = ptr;
	}
	ptr = getF (o, "preludes");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_preludes_cb = ptr;
	}
	ptr = getF (o, "esilcb");
	if (ptr) {
		Py_INCREF (ptr);
		py_arch_esilcb_cb = ptr;
	}
	
	Py_DECREF (o);

	RLibStruct lp = {
		.type = R_LIB_TYPE_ARCH,
		.data = ap,
		.free = (void (*)(void *data))Radare_plugin_core_free
	};
	R_LOG_DEBUG ("PLUGIN[python] Loading arch: %s", meta.name);
	r_lib_open_ptr (Gcore->lib, "python-r_arch.py", NULL, &lp);
	Py_RETURN_TRUE;
}

#endif
