package require Tk

r2cmd "-e scr.color=0"

label .l -text "Command:"
entry .e -width 40 -relief sunken -bd 2 -textvariable name
focus .e
button .b -text Clear -command {
	set output ""
	set name ""
}
button .r -text Run -command {
	set output [r2cmd $name]
	set name ""
}
label .o -text "Output:" -textvariable output -font {-family Courier -size 18} -justify left -wraplength 800
bind .e <Return> {
	set output [r2cmd $name]
	set name ""
}


grid .l -row 0 -column 0 -sticky e
grid .e -row 0 -column 1 -sticky w
grid .r -row 0 -column 2
grid .b -row 0 -column 3
grid .o -row 1 -column 0 -columnspan 3

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
