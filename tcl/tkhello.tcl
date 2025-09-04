package require Tk

# source "/tmp/colorutils.tcl"
# source "/tmp/awthemes.tcl"
# source "/tmp/awclearlooks.tcl"
# source "/tmp/awblack.tcl"

# proc r2cmd {} { return "" }

set ver [r2cmd "?V"]

########### label
set msg [r2cmd "?e Hello From radare2"]
label .hello -text "$msg\n$ver" -anchor w
pack .hello -fill x -expand true

########### textarea
text .txt -width 40 -height 10
pack .txt -side top -fill both -expand true

########### button
proc hello_world {} {
	.txt insert end "console.log('Hello World');\n"
}
button .hello_btn -text "Say Hello" -command hello_world
pack .hello_btn -side left -padx 10 -pady 10

########### runscript button

proc run_script {} {
	set contents [.txt get 1.0 end]
	set script [r2cmd "'?b64 $contents"]
	puts "Running $script"
	r2cmd "'js base64:$script"
}

button .run_btn -text "Run" -command run_script
pack .run_btn -side right -padx 9 -pady 10

# stop script when window is closed. thats what wish do
proc onWindowClose {} {
	set ::done 1
	destroy .
#	exit
}

wm protocol . WM_DELETE_WINDOW onWindowClose
set done 0
# mainloop
vwait ::done
