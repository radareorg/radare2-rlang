require"json"
local inspect = require"inspect"

-- generic utilities
function split(text, sep)
	sep = sep or "\n"
	text = chomp(text)
	local lines = {}
	local pos = 1
	while true do
		local b,e = text:find(sep, pos)
		if not b then table.insert(lines, text:sub(pos)) break end
		table.insert(lines, text:sub(pos,b-1))
		pos = e + 1
	end
	return lines
end
function trim(text)
	if text == nil then return "" end
	text = string.gsub(text, " *$", "")
	text = string.gsub(text, "\n$", "")
	return string.gsub(text, "^ *", "")
end

-- r2api starts here

r2 = {}
function r2.cmd(x)
  return r2cmd(x)
end

function r2.cmdj(x)
  local res = trim(r2cmd(x))
  if res[0] == "[" then assert("UFCK") end
  return json.decode(res)
end
function r2.getConfig(x)
  return r2cmd("e "..x)
end
function r2.setConfig(x, y)
  return r2cmd("e "..x.."="..y)
end
function r2.analyzeProgram()
  return r2cmd("aa")
end
function r2.functionBasicBlocks()
  return split(r2cmd("ablq"),"\n")
end

local inspect = require"inspect"
function r2:ptr (at)
	o = {addr = trim(at)}   -- create object if user does not provide one
	setmetatable(o, self)
	self.__index = self
	o.readBytes = function(self, n)
		return json.decode(r2cmd("p8j "..n.."@"..o.addr))
	end
	o.instruction = function(self)
		return json.decode(r2cmd("aoj@"..o.addr.."~{[0]}"))
	end
	o.add = function(self, x)
		o.addr = trim(r2cmd("?v "..o.addr.."+"..x))
		return self
	end
	return o
end

function hex(x)
	return string.format("0x%08x", x)
end
