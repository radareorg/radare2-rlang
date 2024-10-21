include config.mk

LANGS?=python duktape lua perl vlang

# WIP language support
# LANGS+=csharp
# LANGS+=ruby
# LANGS+=tcc

all: $(LANGS)
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} ; done

install:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} install ; done

install-home:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} install-home ; done

uninstall:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} install-home ; done

uninstall-home:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} uninstall-home ; done

mrproper clean:
	for LANG in $(LANGS); do $(MAKE) -C $${LANG} clean ; done

tests test:
	@echo Nothing to test for now
#	for LANG in $(LANGS); do $(MAKE) -C $${LANG} test ; done

