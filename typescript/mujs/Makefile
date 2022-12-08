all:
	tsc index.ts
	echo "main(r2);" >> index.js
	mv index.js index.mujs
	r2 -qi index.mujs /bin/ls
