include ../config.mk
TCL_FLAGS=$(shell pkg-config --cflags --libs tcl)
R2_FLAGS=$(shell pkg-config --cflags --libs r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)

all:
	$(CC) plugin.c $(TCL_FLAGS) $(R2_FLAGS) -fPIC -shared -o lang_tcl.$(EXT_SO)
	$(MAKE) user-install
	r2 -qi hello.tcl -

include ../install.mk
