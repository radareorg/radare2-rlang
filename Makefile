include config.mk

LANGS?=python duktape

all: $(LANGS)
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} ; done

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

