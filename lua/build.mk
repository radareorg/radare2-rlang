LUAPKG=$(shell pkg-config --list-all|awk '/lua5|lua5-/{print $$1;}')
ifneq (${LUAPKG},)
CFLAGS+=$(shell pkg-config --cflags r_core ${LUAPKG})
LUA_LDFLAGS+=$(shell pkg-config --libs r_lang ${LUAPKG})
else
LUA_VERSION=5.4.4
LUA_CFLAGS+=-Ilua-$(LUA_VERSION)/src
LUA_LDFLAGS+=`ls lua-$(LUA_VERSION)/src/*.c | grep -v lua.c | grep -v luac.c`
endif
CFLAGS+=-Oz
LUA_LDFLAGS+=-Oz -flto

lua lang_lua.${EXT_SO}: lua-sync
	-${CC} $(LUA_CFLAGS) ${CFLAGS} -fPIC ${LDFLAGS_LIB} -o lang_lua.${EXT_SO} lua.c ${LUA_LDFLAGS}

lua-install:
	mkdir -p ${R2PM_PLUGDIR}/lua
	cp -f lang_lua.${EXT_SO} ${R2PM_PLUGDIR}
	cp -f lua/*.lua ${R2PM_PLUGDIR}/lua
	# TODO: move this radare.lua into lua/
	cp -f radare.lua ${R2PM_PLUGDIR}/lua

ifeq (${LUAPKG},)
lua-sync: lua-${LUA_VERSION}
else
lua-sync:
endif

lua-${LUA_VERSION}:
	wget -c https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	tar xzvf lua-$(LUA_VERSION).tar.gz
