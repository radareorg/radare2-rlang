FILES=inspect.lua json.lua r2api.lua

all:
	for a in $(FILES) ; do n=$$(echo $$a|sed -e 's,\.,_,g'); r2 -qfcpc $$a | sed -e "s,buffer,$$n," > $$a.c ; done
