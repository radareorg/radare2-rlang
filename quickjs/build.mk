#QJS_BRANCH=bellard
QJS_BRANCH=frida
VERSION=0.1

LINK_QJS_FILES=1
QJS_LIBC=1
ifeq ($(LINK_QJS_FILES),1)
QJS_FILES+=quickjs-$(QJS_BRANCH)/quickjs.c
QJS_FILES+=quickjs-$(QJS_BRANCH)/cutils.c
QJS_FILES+=quickjs-$(QJS_BRANCH)/libregexp.c
QJS_FILES+=quickjs-$(QJS_BRANCH)/unicode_gen.c
QJS_FILES+=quickjs-$(QJS_BRANCH)/libunicode.c
ifeq ($(QJS_LIBC),1)
QJS_FILES+=quickjs-$(QJS_BRANCH)/quickjs-libc.c
endif
else
QJS_FILES=$(QJS_NAME)/libquickjs.a
endif


ifeq ($(QJS_BRANCH),frida)
	QJS_NAME=quickjs-frida
	QJS_GITURL=https://github.com/frida/quickjs
	QJS_CFLAGS+=-DQJS_NAME="\\"$(QJS_NAME)\\""
	QJS_CFLAGS+=-DQJS_FRIDA=1
else
	QJS_NAME=quickjs-bellard
	QJS_GITURL=https://github.com/bellard/quickjs
	QJS_CFLAGS+=-DQJS_NAME="\\"$(QJS_NAME)\\""
	QJS_CFLAGS+=-DQJS_FRIDA=0
endif

QJS_CFLAGS+=-D QJS_LIBC=$(QJS_LIBC)
QJS_CFLAGS+=-U CONFIG_BIGNUM
QJS_CFLAGS+=-U DUMP_LEAKS
QJS_CFLAGS+=-U NDEBUG

QJS_LIBS+=-lr_core -lr_config -lr_cons

lang_qjs.${EXT_SO}: $(QJS_NAME)
	# $(MAKE) CFLAGS_OPT="-Oz -DCONFIG_VERSION=\\\"0.1\\\"" -C $(QJS_NAME) libquickjs.a
#-Wl,-exported_symbols_list,symbols.lst
	-$(CC) -flto -D 'CONFIG_VERSION="$(VERSION)"' -Os -Wl,-dead_strip -std=c99 $(DUK_CFLAGS) -I quickjs $(QJS_CFLAGS) $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_qjs.$(EXT_SO) $(QJS_FILES) $(QJS_LIBS) qjs.c

$(QJS_NAME):
	git clone $(QJS_GITURL) $(QJS_NAME)

o:
	make clean && make && make user-install
