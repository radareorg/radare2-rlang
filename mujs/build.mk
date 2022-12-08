MUJS_GIT=1
MUJS_VER=1.3.2
MUJS_FILE=mujs-$(MUJS_VER).tar.xz
MUJS_URL=https://mujs.com/downloads/$(MUJS_FILE)
DV=mujs-${MUJS_VER}

ifeq ($(shell type wget 2> /dev/null && echo 1),1)
WGET?=wget -c --no-check-certificate -O
else
WGET?=curl -o
endif

lang_mujs.$(EXT_SO): plugin.o mujs
	-$(CC) -g -std=c99 -flto -Oz $(DUK_CFLAGS) $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_mujs.$(EXT_SO) plugin.c

all: mujs
	$(MAKE) lang_mujs.$(EXT_SO)

mujs mujs-sync:
ifeq ($(MUJS_GIT),1)
	[ ! -d mujs ] && git clone https://github.com/ccxvii/mujs mujs
	[ -d mujs ] && cd mujs && git pull
else
	rm -rf $(MUJS_FILE)
	$(WGET) -O $(MUJS_FILE) $(MUJS_URL)
	tar xJvf $(MUJS_FILE)
	ln -fs mujs-$(MUJS_VER) mujs
endif

mujs-install install-mujs:
	cp -f lang_mujs.${EXT_SO} ${R2PM_PLUGDIR}

mujs-clean:
	rm -f mujs
	rm -rf mujs-$(MUJS_VER)
