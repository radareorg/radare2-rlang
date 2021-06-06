#QJS_BRANCH=bellard
QJS_BRANCH=frida

ifeq ($(QJS_BRANCH),frida)
	QJS_NAME=quickjs-frida
	QJS_GITURL=https://github.com/frida/quickjs
else
	QJS_NAME=quickjs-bellard
	QJS_GITURL=https://github.com/bellard/quickjs
endif

lang_quickjs.${EXT_SO}: $(QJS_NAME)
	$(MAKE) -C $(QJS_NAME) libquickjs.a
	-$(CC) -std=c99 $(DUK_CFLAGS) -I quickjs $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_quickjs.$(EXT_SO) quickjs.c $(QJS_NAME)/libquickjs.a

$(QJS_NAME):
	git clone $(QJS_GITURL) $(QJS_NAME)

