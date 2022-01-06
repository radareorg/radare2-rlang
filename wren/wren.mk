WREN_LIB=wren/libwren.a
# OBJS+=$(WREN_LIB)
#OBJS+=p/wren.o
CFLAGS+=$(shell pkg-config --cflags r_lang r_cons r_util) -Oz
LDFLAGS+=$(shell pkg-config --libs r_lang r_cons r_util)

wren:
	git clone --depth=1 https://github.com/wren-lang/wren
	$(MAKE) $(WREN_LIB)

p/wren.o: p/wren-vm.c p/wren.c

$(WREN_LIB): wren
	cd wren/src/vm/ && $(CC) $(CFLAGS) $(LDFLAGS) -Oz -c *.c -DWREN_OPT_RANDOM=0 -DWREN_OPT_META=0 -I ../include -I ..
	$(AR) rvs $(WREN_LIB) wren/src/vm/*.o
	$(RANLIB) $(WREN_LIB)

PYTHON?=python

p/wren-vm.c: wren
	cd wren && $(PYTHON) util/generate_amalgamation.py > ../p/wren-vm.c
	cp -f wren/src/include/wren.h p/wren-vm.h
