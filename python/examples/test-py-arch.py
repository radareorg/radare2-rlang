# Example Python Arch plugin written in Python
# ============================================
#
#  -- pancake @ nopcode.org
#
# The r2lang.plugin function exposes a way to register new plugins
# into the RCore instance. This API is only available from RLang.
# You must call with with '#!python test.py' or 'r2 -i test.py ..'

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
		if query == R.R_ARCH_INFO_MINOP_SIZE:
			return 1
		if query == R.R_ARCH_INFO_MAXOP_SIZE:
			return 2
		if query == R.R_ARCH_INFO_INVOP_SIZE:  # invalid op size
			return 1
		if query == R.R_ARCH_INFO_CODE_ALIGN:
			return 1
		if query == R.R_ANAL_INFO_DATA_ALIGN:
			return 1
		if query == R.R_ANAL_INFO_DATA2_ALIGN:
			return 2
		if query == R.R_ANAL_INFO_DATA4_ALIGN:
			return 4
		if query == R.R_ANAL_INFO_DATA8_ALIGN:
			return 8
		return 0

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
					"mnemonic" : "mov",
					"type" : R.R_ANAL_OP_TYPE_MOV,
					"cycles" : 2,
					},
				"size": 3
			},
			2: {
				"op": {
					"mnemonic" : "fadd",
					"type" : R.R_ANAL_OP_TYPE_ADD,
					"cpu_family" : R.R_ANAL_OP_FAMILY_FPU,
					"cycles" : 2,
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

if not r2lang.plugin("arch", pyarch):
	print("Failed to register the python arch plugin.")
	
