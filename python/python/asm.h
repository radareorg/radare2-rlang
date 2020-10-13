/* radare - LGPL - Copyright 2017-2019 - pancake, xvilka, aronsky */

#ifndef _PY_ASM_H
#define _PY_ASM_H

#include <r_asm.h>
#include "common.h"

void py_export_asm_enum(PyObject *tp_dict);

void Radare_plugin_asm_free(RAsmPlugin *ap);

PyObject *Radare_plugin_asm(Radare* self, PyObject *args);

#endif /* _PY_ASM_H */
