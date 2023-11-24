package require Tk
set msg [r2cmd "?e Hello From radare2"]
label .hello -text $msg -anchor w
pack .hello -fill x -expand true

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
