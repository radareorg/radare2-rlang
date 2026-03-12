# TCL/TK scripting for radare2

The Tcl binding now exposes an idiomatic `::r2` ensemble:

```tcl
r2 cmd "?V"
r2 print "hello"
r2 plugin core $plugin
set nop $::r2::const::R_ANAL_OP_TYPE_NOP
```

Compatibility aliases are still available: `r2cmd`, `r2plugin`, `r2print`, `r2flush`, `r2const`.

On Debian/Ubuntu:

```
apt install tk-dev
make
```
