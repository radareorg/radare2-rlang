PERLINC=$(shell perl -MConfig -e 'print $$Config{archlib}')/CORE/
lang_perl.${EXT_SO}:
	-${CC} ${CFLAGS} -I$(PERLINC) \
		-fPIC ${LDFLAGS_LIB} -o lang_perl.${EXT_SO} perl.c \
		`perl -MExtUtils::Embed -e ccopts | sed -e 's/-arch [^\s]* //g'` \
		`perl -MExtUtils::Embed -e ldopts | sed -e 's/-arch [^\s]* //g'`
