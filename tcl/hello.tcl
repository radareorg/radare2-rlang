r2cmd "e scr.color=0"
r2cmd "e scr.utf8=0"
set msg [r2cmd "?E Hello World"] 
puts $msg
