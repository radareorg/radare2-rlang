--# require "r2api"
--# require "json"

print("Hello World")
print(r2cmd("?E Hello World"))

local color = Trim(" red ") -- r.api.Config.get("scr.color")
print(color)

-- hello world
local res = r2cmd("afl")
print(res)
