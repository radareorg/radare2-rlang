CC?=gcc
FILES+=lang_tiny.c
FILES+=Tiny/tiny/src/*.c
CFLAGS+=-I Tiny/tiny/include
CFLAGS+=$(shell pkg-config --cflags --libs r_core)
EXT_SO=$(shell r2 -H R2_LIBEXT)
CFLAGS+=-flto -Oz

include ../config.mk

all: Tiny
	$(CC) -shared -o lang_tiny.$(EXT_SO) $(CFLAGS) $(FILES)

Tiny:
	git clone https://github.com/goodpaul6/Tiny


include ../install.mk
