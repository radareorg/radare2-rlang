LANG_NAME=v

$(LANG_NAME) vlang:
	rm -f lang_$(LANG_NAME).$(EXT_SO)
	$(MAKE) lang_$(LANG_NAME).$(EXT_SO)

lang_v.$(EXT_SO):
	$(CC) v.c ${CFLAGS} \
	$(shell pkg-config --cflags --libs r_core r_lang) \
		$(LDFLAGS) $(LDFLAGS_LIB) -fPIC -o lang_$(LANG_NAME).$(EXT_SO)

vlang-clean $(LANG_NAME)-clean:
	rm -f v.o lang_$(LANG_NAME).$(EXT_SO)

vlang-install $(LANG_NAME)-install:
	mkdir -p $(R2PM_PLUGDIR)
	cp -f lang_$(LANG_NAME).$(EXT_SO) ${R2PM_PLUGDIR}
