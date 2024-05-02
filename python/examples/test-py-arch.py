# Example Python Arch plugin written in Python
# ============================================
#
#  -- astuder
#
# To locad the plugin launch radare2 with 'r2 -i test-py-arch.py ..'
# or run '#!python test-py-arch.py' on the radare2 console 

import r2lang
from r2lang import R

def pyarch(a):
	# describes layout of register file for the architecture
	#   return: string with layout of the register file
	def regs():
		return "=PC	pc\n" + \
		"=SP	sp\n" + \
		"=A0	r0\n" + \
		"gpr	r0	.32	0	0\n" + \
		"gpr	r1	.32	4	0\n" + \
		"gpr	r2	.32	8	0\n" + \
		"gpr	r3	.32	12	0\n" + \
		"gpr	r4	.32	16	0\n" + \
		"gpr	r5	.32	20	0\n" + \
		"gpr	sp	.32	24	0\n" + \
		"gpr	pc	.32	28	0\n"

	# provides radare2 with information about instruction size and memory alignment
	#   query: attribute requested
	#   return: value of requested attribute
	def info(query):
		arch_info = {
			R.R_ARCH_INFO_MINOP_SIZE: 1,
			R.R_ARCH_INFO_MAXOP_SIZE: 2,
			R.R_ARCH_INFO_INVOP_SIZE: 1,  # invalid op size
			R.R_ARCH_INFO_CODE_ALIGN: 1,
			R.R_ARCH_INFO_DATA_ALIGN: 1,
			R.R_ARCH_INFO_DATA2_ALIGN: 2,
			R.R_ARCH_INFO_DATA4_ALIGN: 4,
			R.R_ARCH_INFO_DATA8_ALIGN: 8
		}
		res = arch_info.get(query)
		if res is None:
			return 0
		else:
			return res

	# decodes machine code and returns information about the instruction
	#   buf:  buffer with bytes to decode
	#   pc:   memory address where the buffer came from
	#   mask: information requested by radare, use is optional for better performance
	#         see radare2/libr/include/r_arch.h for documentation
	#   return: list object with:
	#     number of bytes processed in from buf to decode instruction
	#     dict with disassembly and metadata about the instruction, most important fields are:
	#       mnemonic: string with disassembled instruction, as shown to user
	#       type: type of instruction
	#       jump: target address for branch instructions
	#       ptr: address for instructions that access memory
	#       see radare2/libr/include/r_anal/op.h for more details and complete list of fields
	def decode(buf, pc, mask):
		ops = {
			0: {
				"op": {
					"mnemonic" : "nop",
					"type" : R.R_ANAL_OP_TYPE_NOP,
					"cycles" : 1,
					},
				"size": 1
			},
			1: {
				"op": {
					"mnemonic" : "mov r{}, [0x{:02x}]".format(buf[1], buf[2]),
					"type" : R.R_ANAL_OP_TYPE_MOV,
					"cycles" : 2,
					"ptr" : buf[2],
					"direction" : R.R_ANAL_OP_DIR_READ,
					},
				"size": 3
			},
			2: {
				"op": {
					"mnemonic" : "fadd r{}, 0x{:02x}".format(buf[1], buf[2]),
					"type" : R.R_ANAL_OP_TYPE_ADD,
					"family" : R.R_ANAL_OP_FAMILY_FPU,
					"cycles" : 2,
					"val" : buf[2],
				},
				"size": 3
			},
			3: {
				"op": {
					"mnemonic" : "jne 0x{:04x}".format((buf[1] << 8) | buf[2]),
					"type" : R.R_ANAL_OP_TYPE_CJMP,
					"cycles" : 2,
					"jump" : (buf[1] << 8) | buf[2],
					"fail" : pc+3,
					"cond" : R.R_ANAL_COND_NE,
					"eob" : True
				},
				"size": 3
			}
		}
		decoded_op = ops.get(buf[0])
		if decoded_op is None:
			return [ 2, None ]
		return [ decoded_op.get("size"), decoded_op.get("op") ]

	# assembles provided string into machine code
	#   addr: memory address where assembled instruction will be located
	#   str: line of code to assemble
	#   return: bytes of assembled code, or None on error
	def encode(addr, str):
		import pyparsing as pp
		p = pp.Word(pp.alphanums) + pp.Optional( pp.Word(pp.alphanums) + pp.Optional(',' + pp.Word(pp.alphanums)))
		asm = p.parseString(str)
		if asm[0] == "nop":
			return b'\x00'
		elif asm[0] == "mov" and len(asm) == 4:
			if asm[1].startswith("r") and asm[3].isdigit():
				return bytes([1, int(asm[1][1:]), int(asm[3])])
		return None

	# definition of the architecture this plugin implements
	#   return: dict with metadata about the plugin, and the functions that implement the plugin
	#       name: pretty name of the plugin
	#       arch: identifier used for referencing architecture in radare, e.g. via: e asm.arch=x
	#       bits: bits of this architecture
	#       regs: layout of register file
	#       info: info about instruction size and memory alignment
	#       decode: disassemble code
	#       encode: assemble code
	return {
		"name": "MyPyArch",
		"arch": "pyarch",
		"bits": 32,
		"license": "GPL",
		"desc": "arch plugin in python",
		"regs": regs,
		"info": info,
		"decode": decode,
		"encode": encode,
	}

# The r2lang.plugin function registers the plugin with radare2
if not r2lang.plugin("arch", pyarch):
	print("Failed to register the python arch plugin.")
