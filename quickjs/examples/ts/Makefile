all:
	tsc --outFile out.js --module amd index.ts r2pipe.ts
	cat boilerplate.js require.js out.js > index.js
	r2 -qc '#!qjs index.js' /bin/ls
