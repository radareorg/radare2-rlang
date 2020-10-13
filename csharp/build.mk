PCP=/Library/Frameworks/Mono.framework/Versions/Current/lib/pkgconfig/

all:
	@echo TODO: csharp

mono csharp lang_csharp.$(EXT_SO):
	$(CC) -fPIC $(LDFLAGS_LIB) -o lang_csharp.$(EXT_SO) \
		$(shell pkg-config --cflags --libs r_util) csharp.c

csharp-install mono-install:
	mkdir -p ${R2PM_PLUGDIR}
	cp -f lang_csharp.$(EXT_SO) ${R2PM_PLUGDIR}

csharp-uninstall mono-uninstall:
	rm -f ${R2PM_PLUGDIR}/lang_csharp.$(EXT_SO)
