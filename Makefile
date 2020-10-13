include config.mk

LANGS?=python duktape

all: $(LANGS)
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} ; done

mrproper clean:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} clean ; done

tests test:
	@echo Nothing to test for now
#	for LANG in $(LANGS); do $(MAKE) -C $${LANG} test ; done

