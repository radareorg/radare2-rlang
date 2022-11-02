WASI_CC=${HOME}/Downloads/wasi/wasi-sdk-16.0/bin/clang 
WASI_SYSROOT=${HOME}/Downloads/wasi/wasi-sysroot-16.0/
WASI_CFLAGS+=--sysroot=${WASI_SYSROOT}
WASI_CFLAGS+=-Wl,--allow-undefined
WASI_CFLAGS+=-shared

all:
	$(WASI_CC) $(WASI_CFLAGS) r2pipe.c -o r2pipe.wasm
	cp -f r2pipe.wasm ../hello.wasm
