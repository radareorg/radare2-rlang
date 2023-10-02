# Example Python Core plugin written in Python
# ===========================================
#  -- pancake 2016-2023
#
# $ r2 -i test-py-core.py -
# > q
# .. hexdump ..
# Dont be rude. Use q!
# > ^D
# $

import r2lang

def pycore(a):
	def _call(s):
		if s == "q":
			try:
				print(r2lang.cmd("x"))
			except:
				print("ERR")
			print("Dont be rude. Use q!")
			return True;
		return False

	return {
		"name": "pycoretest",
		"license": "MIT",
		"desc": "example core plugin written in python",
		"call": _call,
	}

print("Registering Python core plugin...")
print(r2lang.plugin("core", pycore))
