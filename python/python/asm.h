#ifndef _PY_ASM_H
#define _PY_ASM_H

#include <r_asm.h>

#if R2_VERSION_NUMBER < 50800

#include "common.h"

void py_export_asm_enum(PyObject *tp_dict);

void Radare_plugin_asm_free(RAsmPlugin *ap);

PyObject *Radare_plugin_asm(Radare* self, PyObject *args);
#endif

#endif /* _PY_ASM_H */
