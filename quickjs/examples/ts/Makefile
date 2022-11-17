all:
	tsc --outFile out.js --module system index.ts
	cat boilerplate.js out.js > index.js
	r2 -qc '#!qjs index.js' /bin/ls
