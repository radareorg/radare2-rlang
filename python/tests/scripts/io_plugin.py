import r2lang


def pyio(_):
	def _open(path, rw, mode):
		return {"path": path, "mode": mode, "off": 0, "rw": rw}

	def _check(path, many):
		return path.startswith("pytestio://")

	def _read(state, size):
		return b"A" * size

	def _seek(state, offset, whence):
		if whence == 0:
			state["off"] = offset
		elif whence == 1:
			state["off"] += offset
		elif whence == 2:
			state["off"] = 0x40 + offset
		return state["off"]

	def _close(state):
		return 1

	return {
		"name": "pytest-io",
		"license": "MIT",
		"desc": "IO plugin for the Python integration tests",
		"check": _check,
		"open": _open,
		"read": _read,
		"seek": _seek,
		"close": _close,
	}


if not r2lang.plugin("io", pyio):
	raise RuntimeError("failed to register the Python IO test plugin")
