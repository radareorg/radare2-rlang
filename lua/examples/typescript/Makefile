all: node_modules
	tsc index.ts
	node_modules/.bin/castl --babel --strict -o index.tmp index.js
	echo "console={log=print,error=print}" > index.lua
	cat index.tmp | grep -v _ENV | grep -v exports >> index.lua
	echo 'r2 = {cmd=r2cmd,cmdj=r2cmdj}' >> index.lua
	echo 'main(r2)' >> index.lua
	rm -f index.tmp
	r2 -qi index.lua /bin/ls

node_modules:
	mkdir node_modules
	npm i
