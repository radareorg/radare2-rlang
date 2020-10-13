v vlang:
	rm -f lang_v.$(EXT_SO)
	$(MAKE) lang_v.$(EXT_SO)

lang_v.$(EXT_SO):
	${CC} v.c ${CFLAGS} \
	$(shell pkg-config --cflags --libs r_core r_lang) \
	${LDFLAGS} ${LDFLAGS_LIB} -fPIC -o lang_v.$(EXT_SO)

vlang-clean v-clean:
	rm -f v.o lang_v.$(EXT_SO)

vlang-install v-install:
	mkdir -p ${R2PM_PLUGDIR}
	cp -f lang_v.$(EXT_SO) ${R2PM_PLUGDIR}
