# Example Tcl arch plugin using dict-based decode metadata.
# Usage:
#   r2 -qi tcl/test-tcl-arch.tcl -c 'wx 00; e asm.arch=tclarch; e asm.bits=32; pd 1' -

namespace eval ::demo::arch {
    proc regs {} {
        return "=PC\tpc\n=SP\tsp\n=A0\tr0\n=A1\tr1\ngpr\tr0\t.32\t0\t0\ngpr\tr1\t.32\t4\t0\ngpr\tsp\t.32\t8\t0\ngpr\tpc\t.32\t12\t0\n"
    }

    proc decode {op} {
        set bytes [dict get $op bytes]
        binary scan $bytes c opcode
        if {$opcode == 0} {
            return [dict create size 1 mnemonic nop type nop cycles 1]
        }
        return 0
    }
}

set plugin [dict create \
    name "TclArch" \
    arch "tclarch" \
    bits 32 \
    license "MIT" \
    desc "Example arch plugin written in Tcl" \
    regs [list ::demo::arch::regs] \
    info [dict create \
        minop-size 1 \
        maxop-size 1 \
        invop-size 1 \
        code-align 1 \
        data-align 1 \
        data2-align 2 \
        data4-align 4 \
        data8-align 8] \
    decode [list ::demo::arch::decode]]

puts "Registering Tcl arch plugin..."
puts [r2 plugin arch $plugin]
