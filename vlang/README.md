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
