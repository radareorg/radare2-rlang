# zForth for radare2

zforth is a minimalistic implementation of Forth that aims to be easy
to embed, simple and portable.

This plugin exposes the forth interpreter to be used with radare2.

Enter the zforth repl.

```
$ r2 -c '#!zforth' -
```

## Simple programs

```
1 3 + . 10 0 sys ( 10 0 sys is the same as printing \n char to stdout )
```

## Using the r2cmd syscall

```
s" ?E Hello World " r2cmd tell nl
```


