import r2lang
from r2lang import R


def pyarch(_):
	def regs():
		return (
			"=PC\tpc\n"
			"=SP\tsp\n"
			"=A0\tr0\n"
			"gpr\tr0\t.32\t0\t0\n"
			"gpr\tr1\t.32\t4\t0\n"
			"gpr\tr2\t.32\t8\t0\n"
			"gpr\tsp\t.32\t12\t0\n"
			"gpr\tpc\t.32\t16\t0\n"
		)

	def info(query):
		return {
			R.R_ARCH_INFO_MINOP_SIZE: 1,
			R.R_ARCH_INFO_MAXOP_SIZE: 3,
			R.R_ARCH_INFO_INVOP_SIZE: 1,
			R.R_ARCH_INFO_CODE_ALIGN: 1,
			R.R_ARCH_INFO_DATA_ALIGN: 1,
		}.get(query, 0)

	def decode(buf, pc, mask):
		if len(buf) >= 3 and buf[0] == 3:
			jump = (buf[1] << 8) | buf[2]
			return [
				3,
				{
					"mnemonic": f"jne 0x{jump:04x}",
					"type": R.R_ANAL_OP_TYPE_CJMP,
					"cond": R.R_ANAL_CONDTYPE_NE,
					"jump": jump,
					"fail": pc + 3,
					"eob": True,
				},
			]
		if len(buf) >= 3 and buf[0] == 1:
			return [
				3,
				{
					"mnemonic": f"mov r{buf[1]}, 0x{buf[2]:02x}",
					"type": R.R_ANAL_OP_TYPE_MOV,
					"ptr": buf[2],
				},
			]
		return [1, {"mnemonic": "nop", "type": R.R_ANAL_OP_TYPE_NOP}]

	def encode(addr, code):
		if code == "nop":
			return b"\x00"
		if code == "mov r2, 0x33":
			return b"\x01\x02\x33"
		return None

	def fini():
		return True

	return {
		"name": "pytest-arch",
		"arch": "pyarch",
		"bits": 32,
		"license": "MIT",
		"desc": "Arch plugin for the Python integration tests",
		"regs": regs,
		"info": info,
		"decode": decode,
		"encode": encode,
		"fini": fini,
	}


if not r2lang.plugin("arch", pyarch):
	raise RuntimeError("failed to register the Python arch test plugin")
