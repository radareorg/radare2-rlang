lang_quickjs.${EXT_SO}: quickjs
	$(MAKE) -C quickjs libquickjs.a
	-$(CC) -std=c99 $(DUK_CFLAGS) -I quickjs $(CFLAGS) -fPIC $(LDFLAGS_LIB) \
		-o lang_quickjs.$(EXT_SO) quickjs.c quickjs/libquickjs.a
quickjs:
	git clone https://github.com/bellard/quickjs

