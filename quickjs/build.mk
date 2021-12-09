#QJS_BRANCH=bellard
QJS_BRANCH=frida

QJS_LIBC=1

ifeq ($(QJS_BRANCH),frida)
	QJS_NAME=quickjs-frida
	QJS_GITURL=https://github.com/frida/quickjs
	QJS_CFLAGS+=-DQJS_NAME="\\"$(QJS_NAME)\\""
	QJS_CFLAGS+=-DQJS_FRIDA=1
else
	QJS_CFLAGS+=-DQJS_FRIDA=0
	QJS_NAME=quickjs-bellard
	QJS_GITURL=https://github.com/bellard/quickjs
	QJS_CFLAGS+=-DQJS_NAME="\\"$(QJS_NAME)\\""
endif

QJS_CFLAGS+=-DQJS_LIBC=$(QJS_LIBC)

QJS_LIBS+=-lr_core -lr_config

lang_quickjs.${EXT_SO}: $(QJS_NAME)
	$(MAKE) -C $(QJS_NAME) libquickjs.a
	-$(CC) -std=c99 $(DUK_CFLAGS) -I quickjs $(QJS_CFLAGS) $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_quickjs.$(EXT_SO) quickjs.c $(QJS_NAME)/libquickjs.a $(QJS_LIBS)

$(QJS_NAME):
	git clone $(QJS_GITURL) $(QJS_NAME)

