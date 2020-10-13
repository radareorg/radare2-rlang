radare2-rlang
=============

| **GithubCI**  | [![Tests Status](https://github.com/radareorg/radare2-rlang/workflows/R2L/badge.svg)](https://github.com/radareorg/radare2-rlang/actions?query=workflow%3A%22R2L%22)|

RLang plugins are used to instantiate a VM inside radare2 to run scripts by keeping the
internal state alive along multiple executions. This is like interpretting scripts using
r2pipe, but with extra features:

* No need to instantiate and load the libraries on every call
* Keep global state between runs
* Write asm/anal/bin plugins in dynamic languages
* Support Python, Javascript and others!

Building
========

Check for dependencies and build

```
./configure --prefix=/usr
make
make install
```

If you want to build a specific plugin, just cd into the right directory.

```
./configure --prefix=/usr
cd python
make
make install
```
