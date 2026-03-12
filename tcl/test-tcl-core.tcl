# Example Tcl core plugin using the idiomatic ::r2 ensemble.
# Usage:
#   r2 -qi tcl/test-tcl-core.tcl -c tclhello -

namespace eval ::demo::core {
    proc handle {input} {
        if {$input eq "tclhello"} {
            return "Hello from Tcl core plugin!"
        }
        return 0
    }
}

set plugin [dict create \
    name "tclcoretest" \
    license "MIT" \
    desc "Example core plugin written in Tcl" \
    call [list ::demo::core::handle]]

puts "Registering Tcl core plugin..."
puts [r2 plugin core $plugin]
