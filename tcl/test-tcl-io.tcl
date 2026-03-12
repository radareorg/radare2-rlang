# Example Tcl IO plugin using command-prefix callbacks.
# Usage:
#   r2 -qi tcl/test-tcl-io.tcl -c 'o tclio://16; px 8' -

namespace eval ::demo::io {
    proc check {path many} {
        expr {[string match {tclio://*} $path]}
    }

    proc open {path perm mode} {
        dict create path $path size 32
    }

    proc read {state size} {
        string range [string repeat "TCL-IO" 16] 0 [expr {$size - 1}]
    }

    proc seek {state offset whence} {
        set size [dict get $state size]
        switch -- $whence {
            0 { return $offset }
            1 { return $offset }
            2 { return $size }
        }
        return $offset
    }

    proc close {state} {
        return 1
    }
}

set plugin [dict create \
    name "tclio" \
    license "MIT" \
    desc "Example IO plugin written in Tcl" \
    check [list ::demo::io::check] \
    open [list ::demo::io::open] \
    read [list ::demo::io::read] \
    seek [list ::demo::io::seek] \
    close [list ::demo::io::close]]

puts "Registering Tcl IO plugin..."
puts [r2 plugin io $plugin]
