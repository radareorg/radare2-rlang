include ../config.mk
include build.mk
include ../install.mk

test:
	@command -v r2 >/dev/null || { echo "SKIP: r2 not found"; exit 0; }
	@command -v v >/dev/null || { echo "SKIP: V compiler not found"; exit 0; }
	$(MAKE) vlang
	$(MAKE) vlang-install
	@mkdir -p tests
	@echo "Running V plugin smoke test..."
	@r2 -N -q -e scr.color=false -i tests/hello.v - > tests/out.txt 2>&1 || true
	@grep -q "simple hello" tests/out.txt && echo "OK: vlang plugin works" || { echo "FAIL: vlang plugin test failed"; exit 1; }
