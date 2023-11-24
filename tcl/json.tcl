package require json

set data [r2cmd "ij"]
# puts $data

set obj [json::json2dict $data]
set fileName [dict get $obj "core" "file"]
puts "FileName: $fileName"
