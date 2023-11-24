include ../config.mk
TCL_FLAGS=$(shell pkg-config --cflags --libs tcl r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)

all:
	$(CC) $(TCL_FLAGS) -fPIC -shared plugin.c -o lang_tcl.$(EXT_SO)
	$(MAKE) user-install
	r2 -qi hello.tcl -

include ../install.mk
