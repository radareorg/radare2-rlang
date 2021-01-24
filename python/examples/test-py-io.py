# Example Python IO plugin written in Python
# ===========================================
#
#  -- pancake @ nopcode.org
#
# Usage:
#   r2 -I test-py-io.py pyio://33
#
# The r2lang.plugin function exposes a way to register new plugins
# into the RCore instance. This API is only available from RLang.

import r2lang

FAKESIZE = 512

def pyio(a):
	def _open(path, rw, perm):
		print("MyPyIO Opening %s"%(path))
		return "pyio-data"
	def _check(path, many):
		print("python-check %s"%(path))
		return path[0:7] == "pyio://"
	def _read(data, offset, size):
		print("python-read")
		return "A" * size
	def _seek(data, offset, whence):
		print("python-seek")
		if whence == 0: # SET
			return offset
		if whence == 1: # CUR
			return offset
		if whence == 2: # END
			return 512 
		return 512
	def _write(data, offset, buf, size):
		print("python-write")
		return True
	def _system(data, cmd):
		print("python-SYSTEM %s"%(cmd))
		return True
	def _close(data):
		print(data)
		print("python-close")
		return 0
	return {
		"name": "pyio",
		"license": "GPL",
		"desc": "IO plugin in python (pyio://3)",
		"check": _check,
		"open": _open,
		"read": _read,
		"seek": _seek,
		"write": _write,
		"system": _system,
		"close": _close,
	}

print("Registering Python IO plugin...")
print(r2lang.plugin("io", pyio))
