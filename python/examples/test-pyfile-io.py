import r2lang


def pyfile(a):
	def _open(path, rw, perm):
		return open(path[9:], 'rb')

	def _check(path, many):
		return path[0:9] == "pyfile://"

	def _read(data, size):
		return data.read(size)

	def _seek(data, offset, whence):
		data.seek(offset, whence)
		return data.tell()
		
	def _close(data):
		data.close()
		return 0

	return {
		"name": "pyfile",
		"license": "LGPL",
		"desc": "IO plugin in python (pyfile://<path_to_file>)",
		"check": _check,
		"open": _open,
		"read": _read,
		"seek": _seek,
		"close": _close,
	}

r2lang.plugin("io", pyfile)
