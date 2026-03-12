import r2lang

value = r2lang.cmd("?vi 2+3").strip()
if value != "5":
	raise RuntimeError(f"unexpected command result: {value!r}")
print(value)
