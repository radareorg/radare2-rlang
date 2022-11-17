include ../config.mk
include build.mk
include ../install.mk

test:
	r2 -qc '#!qjs examples/world.qjs' /bin/ls
	r2 -qc '#!qjs examples/console.qjs' /bin/ls

.PHONY: test
