include ../config.mk

CFLAGS += -I zForth/src/linux
CFLAGS += -I zForth/src/zforth
CFLAGS += -fPIC -shared
LDFLAGS +=-lr_util -lr_core -lr_config
R2_LIBEXT:=$(shell r2 -H R2_LIBEXT)
R2_USER_PLUGINS=$(shell r2 -H R2_USER_PLUGINS)

all: zForth
	#rax2 -i < zForth/forth/core.zf > core_zf.h
	rax2 -i < core.zf > core_zf.h
	$(CC) -o lang_zforth.$(R2_LIBEXT) $(CFLAGS) $(LDFLAGS) zforth.c zForth/src/zforth/zforth.c
	mkdir -p $(R2_USER_PLUGINS)
	cp -f lang_zforth.$(R2_LIBEXT) $(R2_USER_PLUGINS)

zForth:
	git clone https://github.com/zevv/zForth
