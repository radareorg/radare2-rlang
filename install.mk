clean mrproper:
	-rm -f *.${EXT_SO} *.${EXT_AR} *.o
	-rm -rf *.dSYM

install:
	mkdir -p $(DESTDIR)/$(R2_PLUGIN_PATH)
	[ -n "`ls *.$(EXT_SO)`" ] && cp -f *.$(EXT_SO) $(DESTDIR)/$(R2_PLUGIN_PATH) || true

user-install install-home:
	mkdir -p ${R2PM_PLUGDIR}
	[ -n "`ls *.$(EXT_SO)`" ] && \
		cp -f *.$(EXT_SO) ${R2PM_PLUGDIR} || true

uninstall:
	rm -f $(DESTDIR)/$(R2_PLUGIN_PATH)/"`ls *.$(EXT_SO)`"

user-uninstall uninstall-home:
	rm -f $(R2PM_PLUGDIR)/"`ls *.$(EXT_SO)`"
