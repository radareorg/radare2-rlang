all: libs/zig-wasm3 libs/wasm3
	zig build

run:
	$(MAKE) -C examples
	zig-out/bin/hello

libs/zig-wasm3:
	git clone 'https://github.com/alichay/zig-wasm3' 'libs/zig-wasm3'
	cd libs && patch -p1 < zig-wasm3.patch

libs/wasm3:
	git clone 'https://github.com/wasm3/wasm3' 'libs/wasm3'

clean:
	rm -rf libs/wasm3 libs/zig-wasm3

fmt:
	zig fmt **/*.zig
