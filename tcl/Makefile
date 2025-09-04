include ../config.mk
TCL_FLAGS=$(shell pkg-config --cflags --libs tcl)
R2_FLAGS=$(shell pkg-config --cflags --libs r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)

all:
	$(CC) plugin.c $(TCL_FLAGS) $(R2_FLAGS) -fPIC -shared -o lang_tcl.$(EXT_SO)
	$(MAKE) user-install
	r2 -qi hello.tcl - | grep Hello
	r2 -qi test-tcl-core.tcl -c tclhello - | grep "Hello from TCL"

include ../install.mk
