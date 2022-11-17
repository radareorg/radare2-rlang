all:
	tsc --outFile out.js --module amd index.ts r2pipe.ts
	cat boilerplate.js require.js out.js > index.js
	$(MAKE) types
	r2 -qc '#!qjs index.js' /bin/ls

types:
	tsc --declaration --allowJs --emitDeclarationOnly --outDir types r2pipe.ts
