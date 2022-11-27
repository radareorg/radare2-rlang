FILES=inspect.lua json.lua r2api.lua

all:
	for a in $(FILES) ; do n=$$(echo $$a|sed -e 's,\.,_,g'); printf "const char *$$n = " > $$a.c ; r2 -qfcpcs "$$a" >> $$a.c ; echo ';' >> $$a.c ; done
