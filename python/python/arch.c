/* radare - LGPL - Copyright 2024 - astuder */

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
	PYENUM(R_ARCH_INFO_MINOP_SIZE);
	PYENUM(R_ARCH_INFO_MAXOP_SIZE);
	PYENUM(R_ARCH_INFO_INVOP_SIZE);
	PYENUM(R_ARCH_INFO_CODE_ALIGN);
	PYENUM(R_ARCH_INFO_DATA_ALIGN);
	PYENUM(R_ARCH_INFO_DATA2_ALIGN);
	PYENUM(R_ARCH_INFO_DATA4_ALIGN);
	PYENUM(R_ARCH_INFO_DATA8_ALIGN);

	// opcode type, see radare2/libr/include/r_anal/op.h for documentation	
	PYENUM(R_ARCH_OP_MOD_COND);
	PYENUM(R_ARCH_OP_MOD_REP);
	PYENUM(R_ARCH_OP_MOD_MEM);
	PYENUM(R_ARCH_OP_MOD_REG);
	PYENUM(R_ARCH_OP_MOD_IND);
	PYENUM(R_ANAL_OP_TYPE_NULL);
	PYENUM(R_ANAL_OP_TYPE_JMP);
	PYENUM(R_ANAL_OP_TYPE_UJMP);
	PYENUM(R_ANAL_OP_TYPE_RJMP);
	PYENUM(R_ANAL_OP_TYPE_UCJMP);
	PYENUM(R_ANAL_OP_TYPE_IJMP);
	PYENUM(R_ANAL_OP_TYPE_IRJMP);
	PYENUM(R_ANAL_OP_TYPE_CJMP);
	PYENUM(R_ANAL_OP_TYPE_MJMP);
	PYENUM(R_ANAL_OP_TYPE_RCJMP);
	PYENUM(R_ANAL_OP_TYPE_MCJMP);
	PYENUM(R_ANAL_OP_TYPE_CALL);
	PYENUM(R_ANAL_OP_TYPE_UCALL);
	PYENUM(R_ANAL_OP_TYPE_RCALL);
	PYENUM(R_ANAL_OP_TYPE_ICALL);
	PYENUM(R_ANAL_OP_TYPE_IRCALL);
	PYENUM(R_ANAL_OP_TYPE_CCALL);
	PYENUM(R_ANAL_OP_TYPE_UCCALL);
	PYENUM(R_ANAL_OP_TYPE_RET);
	PYENUM(R_ANAL_OP_TYPE_CRET);
	PYENUM(R_ANAL_OP_TYPE_ILL);
	PYENUM(R_ANAL_OP_TYPE_UNK);
	PYENUM(R_ANAL_OP_TYPE_NOP);
	PYENUM(R_ANAL_OP_TYPE_MOV);
	PYENUM(R_ANAL_OP_TYPE_CMOV);
	PYENUM(R_ANAL_OP_TYPE_TRAP);
	PYENUM(R_ANAL_OP_TYPE_SWI);
	PYENUM(R_ANAL_OP_TYPE_CSWI);
	PYENUM(R_ANAL_OP_TYPE_UPUSH);
	PYENUM(R_ANAL_OP_TYPE_RPUSH);
	PYENUM(R_ANAL_OP_TYPE_PUSH);
	PYENUM(R_ANAL_OP_TYPE_POP);
	PYENUM(R_ANAL_OP_TYPE_CMP);
	PYENUM(R_ANAL_OP_TYPE_ACMP);
	PYENUM(R_ANAL_OP_TYPE_ADD);
	PYENUM(R_ANAL_OP_TYPE_SUB);
	PYENUM(R_ANAL_OP_TYPE_IO);
	PYENUM(R_ANAL_OP_TYPE_MUL);
	PYENUM(R_ANAL_OP_TYPE_DIV);
	PYENUM(R_ANAL_OP_TYPE_SHR);
	PYENUM(R_ANAL_OP_TYPE_SHL);
	PYENUM(R_ANAL_OP_TYPE_SAL);
	PYENUM(R_ANAL_OP_TYPE_SAR);
	PYENUM(R_ANAL_OP_TYPE_OR);
	PYENUM(R_ANAL_OP_TYPE_AND);
	PYENUM(R_ANAL_OP_TYPE_XOR);
	PYENUM(R_ANAL_OP_TYPE_NOR);
	PYENUM(R_ANAL_OP_TYPE_NOT);
	PYENUM(R_ANAL_OP_TYPE_STORE);
	PYENUM(R_ANAL_OP_TYPE_LOAD);
	PYENUM(R_ANAL_OP_TYPE_LEA);
	PYENUM(R_ANAL_OP_TYPE_ULEA);
	PYENUM(R_ANAL_OP_TYPE_LEAVE);
	PYENUM(R_ANAL_OP_TYPE_ROR);
	PYENUM(R_ANAL_OP_TYPE_ROL);
	PYENUM(R_ANAL_OP_TYPE_XCHG);
	PYENUM(R_ANAL_OP_TYPE_MOD);
	PYENUM(R_ANAL_OP_TYPE_SWITCH);
	PYENUM(R_ANAL_OP_TYPE_CASE);
	PYENUM(R_ANAL_OP_TYPE_LENGTH);
	PYENUM(R_ANAL_OP_TYPE_CAST);
	PYENUM(R_ANAL_OP_TYPE_NEW);
	PYENUM(R_ANAL_OP_TYPE_ABS);
	PYENUM(R_ANAL_OP_TYPE_CPL);
	PYENUM(R_ANAL_OP_TYPE_CRYPTO);
	PYENUM(R_ANAL_OP_TYPE_SYNC);
	PYENUM(R_ANAL_OP_TYPE_DEBUG);

	// opcode condition, see radare2/libr/include/r_anal/op.h for documentation
	PYENUM(R_ANAL_COND_AL);
	PYENUM(R_ANAL_COND_EQ);
	PYENUM(R_ANAL_COND_NE);
	PYENUM(R_ANAL_COND_GE);
	PYENUM(R_ANAL_COND_GT);
	PYENUM(R_ANAL_COND_LE);
	PYENUM(R_ANAL_COND_LT);
	PYENUM(R_ANAL_COND_NV);
	PYENUM(R_ANAL_COND_HS);
	PYENUM(R_ANAL_COND_LO);
	PYENUM(R_ANAL_COND_MI);
	PYENUM(R_ANAL_COND_PL);
	PYENUM(R_ANAL_COND_VS);
	PYENUM(R_ANAL_COND_VC);
	PYENUM(R_ANAL_COND_HI);
	PYENUM(R_ANAL_COND_LS);

	// opcode family, see radare2/libr/include/r_anal/op.h for documentation	
	PYENUM(R_ANAL_OP_FAMILY_UNKNOWN);
	PYENUM(R_ANAL_OP_FAMILY_CPU);
	PYENUM(R_ANAL_OP_FAMILY_FPU);
	PYENUM(R_ANAL_OP_FAMILY_VEC);
	PYENUM(R_ANAL_OP_FAMILY_PRIV);
	PYENUM(R_ANAL_OP_FAMILY_CRYPTO);
	PYENUM(R_ANAL_OP_FAMILY_THREAD);
	PYENUM(R_ANAL_OP_FAMILY_VIRT);
	PYENUM(R_ANAL_OP_FAMILY_SECURITY);
	PYENUM(R_ANAL_OP_FAMILY_IO);
	PYENUM(R_ANAL_OP_FAMILY_SIMD);

#undef PYENUM
}

static bool py_arch_init(RArchSession *as) {
	// is this needed/called?
	r_return_val_if_fail (as, false);
	// R_LOG_WARN ("py_arch_init not implemented");
	return true;
}

static bool py_arch_fini(RArchSession *as) {
	// is this needed/called?
	r_return_val_if_fail (as, false);
	// R_LOG_WARN ("py_arch_fini not implemented");
	return true;
}

static int py_arch_info(RArchSession *as, ut32 q) {
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
		} else {
			PyErr_Print();
		}
		Py_DECREF (arglist);
	}
	return res;
}

static char *py_arch_regs(RArchSession *as) {
	r_return_val_if_fail (as, NULL);
//	R_LOG_INFO ("py_arch_regs called");
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
		} else {
			PyErr_Print();
		}
	}
	return res;
}

static bool py_arch_encode(RArchSession *as, RAnalOp *op, RArchEncodeMask mask) {
	r_return_val_if_fail (as && op, false);
	// TODO
//	R_LOG_WARN ("py_arch_encode not implemented");
	return true;
}

static bool py_arch_decode(RArchSession *as, RAnalOp *op, RAnalOpMask mask) {
	r_return_val_if_fail (as && op, false);
	// WIP
//	R_LOG_INFO ("py_arch_decode called");
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
		PyObject *arglist = Py_BuildValue ("(NK)", memview, op->addr);
		PyObject *result = PyObject_CallFunction (py_arch_decode_cb, "NK", memview, op->addr);
		if (result) {
			if (PyList_Check (result)) {
				PyObject *len = PyList_GetItem (result, 0);
				PyObject *dict = PyList_GetItem (result, 1);
				if (len && dict) {
					if (PyDict_Check (dict)) {
						op->size = PyNumber_AsSsize_t (len, NULL);
						op->mnemonic = strdup (getS (dict, "mnemonic"));
						op->family = (getI (dict, "family"));
						op->cycles = (getI (dict, "cycles"));
						op->type = (getI (dict, "type"));
						op->jump = (getI (dict, "jump"));
						op->fail = (getI (dict, "fail"));
						op->eob = (getB (dict, "eob"));
						res = true;
					}
				}
			}
			Py_DECREF (result);
		} 
		Py_DECREF (arglist);
		Py_DECREF (memview);
	} else {
		R_LOG_WARN ("arch plugin does not implement decode");
	}

	if (PyErr_Occurred ()) {
		PyErr_Print ();
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
	// TODO
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
	r_lib_open_ptr (core->lib, "python-r_arch.py", NULL, &lp);
	Py_RETURN_TRUE;
}

#endif
