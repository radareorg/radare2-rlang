all: require.js
	tsc --outFile out.js --module amd index.ts r2pipe.ts examples.ts
	cat boilerplate.js require.js out.js > index.js
	-pkill -USR1 r2

require.js:
	# wget ...

vs:
	open -a "Visual Studio Code" . index.ts

r2:
	r2 -e cmd.usr1='#!qjs index.js' /bin/ls

other:
	$(MAKE) types
	r2 -qc '#!qjs index.js' /bin/ls

types:
	tsc --declaration --allowJs --emitDeclarationOnly --outDir types r2pipe.ts
.PHONY: types
