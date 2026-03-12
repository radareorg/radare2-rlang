#ifndef TCL_PLUGIN_H
#define TCL_PLUGIN_H

#include <r_core.h>
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

char *tcl_strdup0(const char *s);
void tcl_reset_result(Tcl_Interp *interp);
void tcl_install_constants(Tcl_Interp *interp);
int tcl_get_dict_value(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, Tcl_Obj **out);
Tcl_Obj *tcl_dup_obj(Tcl_Obj *obj);
void tcl_obj_unref(Tcl_Obj *obj);
int tcl_eval_prefix(Tcl_Interp *interp, Tcl_Obj *prefix, int argc, Tcl_Obj *const argv[], Tcl_Obj **result);
Tcl_Obj *tcl_resolve_provider(Tcl_Interp *interp, Tcl_Obj *provider);
char *tcl_dict_dup_string(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required);
Tcl_Obj *tcl_dict_dup_callback(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key, bool required);
bool tcl_obj_to_boolish(Tcl_Interp *interp, Tcl_Obj *obj, bool *out);
bool tcl_obj_to_wide(Tcl_Interp *interp, Tcl_Obj *obj, Tcl_WideInt *out);
ut32 tcl_dict_get_u32(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key);
ut32 tcl_dict_get_endian(Tcl_Interp *interp, Tcl_Obj *dictObj, const char *key);
TclPluginContext *tcl_context(RCore *core);
RList *tcl_ensure_plugin_list(RList **list, RListFree freefn);
int tcl_open_plugin(TclPluginContext *ctx, const char *name, int type, void *data, void(*freefn)(void *));

int r2_cmd_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int r2_print_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int r2_flush_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int r2_const_objcmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);

int tcl_register_core_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec);
int tcl_register_io_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec);
int tcl_register_arch_plugin(TclPluginContext *ctx, Tcl_Interp *interp, Tcl_Obj *spec);

#endif
