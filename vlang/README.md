Vlang support for Rlang
=======================

This plugin allows you to run V scripts inside r2:

```go
$ cat example.v
import os

pub fn entry(core &R2) {
	println("simple hello")
	println(core.cmd('?E hello'))
	os.system('echo world')
}
```

Then you can run it like this:

```
$ r2 -i example.v -
hello
 .--.     .-------.
 | _|     |       |
 | O O   <  hello |
 |  |  |  |       |
 || | /   `-------'
 |`-'|
 `---'

world
[0x00000000]> q
```

Pending stuff:
--------------

* [ ] Support cmdj() with x.json2 or so
* [ ] Basic r2pipe-api on top of it
* [x] Support writing core plugins in V-rlang

Core plugins in V
-----------------

Register a core plugin from V so that it can handle commands entered in r2:

```
// vlang/tests/test-core-vlang.v
pub fn vcore_call(core &R2, input byteptr) bool {
    unsafe {
        if input.vstring() == 'vhello' {
            println(core.cmd('x 16 @ $$'))
            println('Hello from V core plugin!')
            return true
        }
    }
    return false
}

pub fn entry(core &R2) {
    _ = core.register_core('vcoretest', vcore_call, 'Example core plugin written in V', 'MIT')
}

// Run: r2 -qi vlang/tests/test-core-vlang.v -c vhello -
```
