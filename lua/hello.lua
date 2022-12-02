--# require "r2api"
--# require "json"

print("Hello World")
print(r2cmd("?E Hello World"))

function foo()
	print(r2.cmdj("x"))
end
xpcall(foo,print)

print("DONE")
-- hello world
local res = r2cmd("afl")
print(res)
