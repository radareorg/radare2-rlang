MUJS_VER=1.3.2
MUJS_FILE=mujs-$(MUJS_VER).tar.xz
MUJS_URL=https://mujs.com/downloads/$(MUJS_FILE)
DV=mujs-${MUJS_VER}


lang_mujs.$(EXT_SO): plugin.o mujs
	-$(CC) -g -std=c99 -flto -Oz $(DUK_CFLAGS) $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_mujs.$(EXT_SO) plugin.c

all: mujs
	$(MAKE) lang_mujs.$(EXT_SO)

mujs mujs-sync:
	rm -f $(MUJS_FILE)
	wget -O $(MUJS_FILE) $(MUJS_URL)
	tar xJvf $(MUJS_FILE)
	ln -fs mujs-$(MUJS_VER) mujs

mujs-install install-mujs:
	cp -f lang_mujs.${EXT_SO} ${R2PM_PLUGDIR}

mujs-clean:
	rm -f mujs
	rm -rf mujs-$(MUJS_VER)
