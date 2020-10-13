ifeq ($(OSTYPE),windows)
lang_python.${EXT_SO}:
	${CC} ${CFLAGS} -I${HOME}/.wine/drive_c/Python37/include \
	-L${HOME}/.wine/drive_c/Python37/libs \
	$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --cflags --libs r_reg r_core r_cons) \
	${LDFLAGS_LIB} -o lang_python.${EXT_SO} python.c -lpython37
else
PYCFLAGS=$(shell PYVER=3 ./python-config-wrapper --includes) -DPYVER=3
# Python3.8+ requires new `--embed` flag to include `-lpython`. 
PYLDFLAGS=$(shell PYVER=3 ./python-config-wrapper --libs --embed || \
                  PYVER=3 ./python-config-wrapper --libs)
PYLDFLAGS+=$(shell PYVER=3 ./python-config-wrapper --ldflags)
PYLDFLAGS+=${LDFLAGS_LIB}

lang_python.$(EXT_SO):
	${CC} python.c python/*.c ${CFLAGS} ${PYCFLAGS} ${PYLDFLAGS} \
	$(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --cflags --libs r_reg r_core r_cons) \
	${LDFLAGS} ${LDFLAGS_LIB} -fPIC -o lang_python.$(EXT_SO)
endif

py python:
	rm -f lang_python.$(EXT_SO)
	$(MAKE) lang_python.$(EXT_SO)

py-clean:
	rm -f python.o lang_python.$(EXT_SO)

py-install python-install:
	mkdir -p ${R2PM_PLUGDIR}
	cp -f lang_python.$(EXT_SO) ${R2PM_PLUGDIR}

py-uninstall python-uninstall:
	rm -f ${R2PM_PLUGDIR}/lang_python.$(EXT_SO)
