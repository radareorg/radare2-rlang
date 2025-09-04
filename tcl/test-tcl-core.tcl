# Example TCL Core plugin written in TCL
# Usage:
#   r2 -qi tcl/test-tcl-core.tcl -
# Then type 'tclhello' in the r2 prompt to test.

proc tclcore_provider {} {
    return [dict create \
        name "tclcoretest" \
        license "MIT" \
        desc "Example core plugin written in TCL" \
        call tclcore_call]
}

proc tclcore_call {input} {
    if {$input eq "tclhello"} {
        # run an r2 command from TCL and print output
        puts [r2cmd "x 16 @ $$"]
        puts "Hello from TCL core plugin!"
        return 1
    }
    return 0
}

puts "Registering TCL core plugin..."
puts [r2plugin core tclcore_provider]
