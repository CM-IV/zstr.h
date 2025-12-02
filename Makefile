
# Configuration.
BUNDLER = z-core/zbundler.py
SRC_CORE = src/zstr.c
DIST_HEADER = zstr.h

# Lua Configuration (Adjust versions as needed: lua5.3, lua5.4, luajit).
LUA_VERSION = lua5.4
LUA_INCLUDE = /usr/include/$(LUA_VERSION)
LUA_LIB = $(LUA_VERSION)
LUA_SRC = bindings/lua/zstr_module.c
LUA_OUT = zstr.so

# Default target.
all: bundle lua

bundle:
	@echo "Bundling $(DIST_HEADER)..."
	@mkdir -p include
	python3 $(BUNDLER) $(SRC_CORE) $(DIST_HEADER) 

lua: $(DIST_HEADER) $(LUA_SRC)
	@echo "Building Lua binding ($(LUA_OUT))..."
	gcc -O3 -shared -fPIC -o $(LUA_OUT) $(LUA_SRC) -I. -I$(LUA_INCLUDE) -l$(LUA_LIB) 

clean:
	rm -f $(LUA_OUT)

init:
	git submodule update --init --recursive

.PHONY: all bundle lua clean init example_lua
