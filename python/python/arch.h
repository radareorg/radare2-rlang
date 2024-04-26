/* radare - LGPL - Copyright 2024 - astuder */

#ifndef _PY_ARCH_H
#define _PY_ARCH_H

#include <r_arch.h>
#include "common.h"

void py_export_arch_enum(PyObject *tp_dict);

void Radare_plugin_arch_free(RArchPlugin *ap);

PyObject *Radare_plugin_arch(Radare* self, PyObject *args);

#endif /* _PY_ARCH_H */