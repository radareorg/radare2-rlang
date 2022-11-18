require"r2api"
local inspect = require"inspect"

-- function r2cmd() end

local size = 4
local res = r2cmd("px " .. size)
local entry0 = r2:ptr("entry0")

local pc = entry0

-- disasm until invalid instruction
while true do
	local op = pc:instruction()

	if op.opcode == "invalid" then break end
	print(hex(op.addr).."   "..op.size.."  "..op.opcode)
	pc:add(op.size)
end

r2.analyzeProgram()
local bytes = r2:ptr("entry0"):readBytes(4)
print(inspect(bytes))
local arch = r2.getConfig("asm.arch")
print(arch)
print(res)
print("Hello World")
