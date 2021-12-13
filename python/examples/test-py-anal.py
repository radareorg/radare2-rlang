# Example Python Anal plugin written in Python
# ============================================
#
#  -- pancake @ nopcode.org
#
# The r2lang.plugin function exposes a way to register new plugins
# into the RCore instance. This API is only available from RLang.
# You must call with with '#!python test.py' or 'r2 -i test.py ..'

import r2lang
from r2lang import R

def pyanal(a):
	def set_reg_profile():
		return "=PC	pc\n" + \
		"=SP	sp\n" + \
		"gpr	r0	.32	0	0\n" + \
		"gpr	r1	.32	4	0\n" + \
		"gpr	r2	.32	8	0\n" + \
		"gpr	r3	.32	12	0\n" + \
		"gpr	r4	.32	16	0\n" + \
		"gpr	r5	.32	20	0\n" + \
		"gpr	sp	.32	24	0\n" + \
		"gpr	pc	.32	28	0\n"

	def archinfo(query):
		if query == R.R_ANAL_ARCHINFO_MIN_OP_SIZE:
			return 2
		if query == R.R_ANAL_ARCHINFO_MAX_OP_SIZE:
			return 2
		if query == R.R_ANAL_ARCHINFO_INV_OP_SIZE:  # invalid op size
			return 2
		if query == R.R_ANAL_ARCHINFO_ALIGN:
			return 2
		if query == R.R_ANAL_ARCHINFO_DATA_ALIGN:
			return 2
		return 0

	def analop(buf, pc):
		op = {
			"type" : R.R_ANAL_OP_TYPE_NULL,
			"cycles" : 0,
			"stackop" : 0,
			"stackptr" : 0,
			"ptr" : -1,
			"jump" : -1,
			"addr" : 0,
			"eob" : False,
			"esil" : "",
		}
		return [ 2, op ]

	return {
		"name": "MyPyAnal",
		"arch": "pyarch",
		"bits": 32,
		"license": "GPL",
		"desc": "anal plugin in python",
		"set_reg_profile": set_reg_profile,
		"archinfo": archinfo,
		"op": analop,
	}

if not r2lang.plugin("anal", pyanal):
	print("Failed to register the python anal plugin.")
	
