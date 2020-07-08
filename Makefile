R2_PLUGIN_PATH=$(shell r2 -H R2_USER_PLUGINS)
R2_LIBDIR_PATH=$(shell r2 -H R2_LIBDIR)
PKG_CONFIG_PATH=${R2_LIBDIR_PATH}/pkgconfig
CFLAGS+=$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --cflags r_core)
CFLAGS+=-DPREFIX=\"${PREFIX}\"

DUK_CFLAGS+=-Wall -DPREFIX=\"${PREFIX}\" -I. -Iduk

R2PM_PLUGDIR?=${R2_PLUGIN_PATH}
EXT_SO?=$(shell r2 -H LIBEXT)

ifeq ($(EXT_SO),)
ifeq ($(OSTYPE),darwin)
CFLAGS+=-undefined dynamic_lookup
EXT_SO=dylib
else
ifeq ($(OSTYPE),windows)
EXT_SO=dll
else
EXT_SO=so
endif
endif
endif

LDFLAGS_LIB=$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --libs-only-L r_core) -lr_core -lr_io -lr_util -shared -lr_asm

LANGS?=py duktape

all: $(LANGS)
	@echo "LANG ${LANGS}"

ifeq ($(OSTYPE),windows)
lang_python.${EXT_SO}:
	${CC} ${CFLAGS} -I${HOME}/.wine/drive_c/Python37/include \
	-L${HOME}/.wine/drive_c/Python37/libs \
	$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --cflags --libs r_reg r_core r_cons) \
	${LDFLAGS_LIB} -o lang_python.${EXT_SO} python.c -lpython37
else
PYCFLAGS=$(shell PYVER=3 ./python-config-wrapper --includes) -DPYVER=3
# Python3.8+ requires new `--embed` flag to include `-lpython`. 
PYLDFLAGS=$(shell PYVER=3 ./python-config-wrapper --libs --embed || \
                  PYVER=3 ./python-config-wrapper --libs)
PYLDFLAGS+=$(shell PYVER=3 ./python-config-wrapper --ldflags)
PYLDFLAGS+=${LDFLAGS_LIB}

lang_python.$(EXT_SO):
	${CC} python.c python/*.c ${CFLAGS} ${PYCFLAGS} ${PYLDFLAGS} \
	$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --cflags --libs r_reg r_core r_cons) \
	${LDFLAGS} ${LDFLAGS_LIB} -fPIC -o lang_python.$(EXT_SO)
endif

py python:
	rm -f lang_python.$(EXT_SO)
	$(MAKE) lang_python.$(EXT_SO)

py-clean:
	rm -f python.o lang_python.$(EXT_SO)

py-install python-install:
	mkdir -p ${R2PM_PLUGDIR}
	cp -f lang_python.$(EXT_SO) ${R2PM_PLUGDIR}

py-uninstall python-uninstall:
	rm -f ${R2PM_PLUGDIR}/lang_python.$(EXT_SO)

duktape:
	$(MAKE) lang_duktape.$(EXT_SO)

lang_duktape.$(EXT_SO): duktape.o duk
	-$(CC) -std=c99 $(DUK_CFLAGS) $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_duktape.$(EXT_SO) duktape.c

mrproper clean:
	-rm -f *.${EXT_SO} *.${EXT_AR} *.o
	-rm -rf *.dSYM

install:
	mkdir -p $(DESTDIR)/$(R2_PLUGIN_PATH)
	[ -n "`ls *.$(EXT_SO)`" ] && cp -f *.$(EXT_SO) $(DESTDIR)/$(R2_PLUGIN_PATH) || true

install-home:
	mkdir -p ${R2PM_PLUGDIR}
	[ -n "`ls *.$(EXT_SO)`" ] && \
		cp -f *.$(EXT_SO) ${R2PM_PLUGDIR} || true

DUKTAPE_VER=2.4.0
DUKTAPE_FILE=duktape-$(DUKTAPE_VER).tar.xz
DUKTAPE_URL=https://duktape.org/$(DUKTAPE_FILE)
DV=duktape-${DUKTAPE_VER}

p:
	rm -f lang_python.${EXT_SO}
	$(MAKE) lang_python.${EXT_SO} PYVER=3
	cp -f lang_python.${EXT_SO} ~/.local/share/radare2/plugins


duk duktape-sync duk-sync sync-dunk sync-duktape:
	rm -f $(DUKTAPE_FILE)
	wget -O $(DUKTAPE_FILE) $(DUKTAPE_URL)
	tar xJvf $(DUKTAPE_FILE)
	mkdir -p duk
	cp -f $(DV)/src/duktape.* duk/
	cp -f $(DV)/src/duk_config.h duk/
	cp -f $(DV)/extras/console/duk* duk/
	rm -rf $(DUKTAPE_FILE) ${DV}

duk-install install-duk:
	cp -f lang_duktape.${EXT_SO} ${R2PM_PLUGDIR}

