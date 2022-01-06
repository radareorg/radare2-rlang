include ../config.mk

WREN_PLUGIN=lang_wren.$(EXT_SO)

all: $(WREN_PLUGIN)

$(WREN_PLUGIN): wren/libwren.a p/wren.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC p/wren.c -o lang_wren.$(EXT_SO)

clean:
	rm -f p/*.o *.$(EXT_SO)

mrproper: clean
	rm -rf wren

user-install:
	cp -f lang_wren.$(EXT_SO) $(R2PM_PLUGDIR)/lang_wren.$(EXT_SO)

user-uninstall:
	rm -f $(R2PM_PLUGDIR)/lang_wren.$(EXT_SO)

include wren.mk
