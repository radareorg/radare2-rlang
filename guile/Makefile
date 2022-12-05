include ../config.mk
GUILE_FLAGS=$(shell pkg-config guile-3.0 --cflags --libs guile-3.0 r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)

all:
	$(CC) $(GUILE_FLAGS) -fPIC -shared plugin.c -o lang_guile.$(EXT_SO)
	$(MAKE) user-install
	r2 -qi hello.scm -

include ../install.mk
