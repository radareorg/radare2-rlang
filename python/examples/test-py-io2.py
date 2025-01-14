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

FAKESIZE = 100

def pyio(a):
	def _open(path, rw, perm):
		print("MyPyIO2 Opening %s"%(path))
		return "pyio-data"
	def _check(path, many):
		print("python-check %s"%(path))
		return path[0:8] == "pyio2://"
	def _read(data, size):
		print("python-read")
		return "B" * size
	def _seek(data, offset, whence):
		print("python-seek")
		if whence == 0: # SET
			return offset
		if whence == 1: # CUR
			return offset
		if whence == 2: # END
			return FAKESIZE
		return FAKESIZE
	def _write(data, buf, size):
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
		"name": "pyio2",
		"license": "GPL",
		"desc": "IO plugin in python (pyio2://3)",
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
