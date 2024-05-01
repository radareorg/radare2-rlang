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

	def decode(buf, pc):
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
					"mnemonic" : "mov r{}, 0x{:02x}".format(buf[1], buf[2]),
					"type" : R.R_ANAL_OP_TYPE_MOV,
					"cycles" : 2,
					"ptr" : buf[2],
					"direction" : R.R_ANAL_OP_DIR_READ,
					},
				"size": 3
			},
			2: {
				"op": {
					"mnemonic" : "fadd r{}, #0x{:02x}".format(buf[1], buf[2]),
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

	return {
		"name": "MyPyArch",
		"arch": "pyarch",
		"bits": 32,
		"license": "GPL",
		"desc": "arch plugin in python",
		"regs": regs,
		"info": info,
		"decode": decode,
	}

# The r2lang.plugin function register the plugin with radare2
if not r2lang.plugin("arch", pyarch):
	print("Failed to register the python arch plugin.")
	
