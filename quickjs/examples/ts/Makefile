all: require.js
	tsc --outFile out.js --module amd hello.ts
# tsc --outFile out.js --module amd index.ts r2pipe.ts examples.ts
	cat require.js out.js boilerplate.js > index.qjs
	r2 -qi index.qjs /bin/ls
#-pkill -USR1 r2

require.js:
	# wget ...

vs:
	open -a "Visual Studio Code" . index.ts

r2:
	r2 -e cmd.usr1='. index.qjs' /bin/ls

other:
	$(MAKE) types
	r2 -qc '#!qjs index.qjs' /bin/ls

types:
	tsc --declaration --allowJs --emitDeclarationOnly --outDir types r2pipe.ts
.PHONY: types
