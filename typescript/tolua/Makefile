all:
	rm -f index.lua lualib_bundle.lua
	tstl -lt 5.4 --luaBundle index.lua --luaBundleEntry index.ts index.ts
	sed -i -e 's/return require("index", ...)/return require("index", ...)["main"](nil, r2)/' index.lua
	sed -i -e 's/JSON:/json./g' index.lua
	sed -i -e 's/self.r2:cmd/r2cmd/g' index.lua
	sed -i -e 's/r2:cmd/r2cmd/g' index.lua
	r2 -qi index.lua /bin/ls
