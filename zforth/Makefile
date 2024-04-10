include ../config.mk

CFLAGS += -I zForth/src/linux
CFLAGS += -I zForth/src/zforth
CFLAGS += -fPIC -shared
LDFLAGS +=-lr_util
R2_LIBEXT:=$(shell r2 -H R2_LIBEXT)
R2_USER_PLUGINS=$(shell r2 -H R2_USER_PLUGINS)

all: zForth
	$(CC) -o lang_zforth.$(R2_LIBEXT) $(CFLAGS) $(LDFLAGS) zforth.c zForth/src/zforth/zforth.c
	mkdir -p $(R2_USER_PLUGINS)
	cp -f lang_zforth.$(R2_LIBEXT) $(R2_USER_PLUGINS)

zForth:
	git clone https://github.com/zevv/zForth
