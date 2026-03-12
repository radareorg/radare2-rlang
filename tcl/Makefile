include ../config.mk
TCL_FLAGS=$(shell pkg-config --cflags --libs tcl)
R2_FLAGS=$(shell pkg-config --cflags --libs r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)
SRCS=plugin_common.c plugin_core.c plugin_io.c plugin_arch.c plugin_lang.c

all:
	$(CC) $(SRCS) $(TCL_FLAGS) $(R2_FLAGS) -fPIC -shared -o lang_tcl.$(EXT_SO)
	$(MAKE) user-install
	r2 -qi hello.tcl - | grep Hello
	r2 -qi test-tcl-core.tcl -c tclhello - | grep "Hello from Tcl core plugin!"
	r2 -qi test-tcl-io.tcl -c 'o tclio://16; p8 4' - | grep 54434c2d
	r2 -qi test-tcl-arch.tcl -c 'e scr.color=0; wx 00; e asm.arch=tclarch; e asm.bits=32; pd 1' - | grep nop

include ../install.mk
