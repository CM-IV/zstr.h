
# Configuration
BUNDLER     = z-core/zbundler.py
SRC_CORE    = src/zstr.c
DIST_HEADER = zstr.h

# Standard Lua (Default: 5.4)
LUA_STD_VER = lua5.4
LUA_STD_INC = /usr/include/$(LUA_STD_VER)
LUA_STD_LIB = $(LUA_STD_VER)

# LuaJIT (Manual command settings)
LUA_JIT_INC = /usr/include/luajit-2.1
LUA_JIT_LIB = luajit-5.1

# Source/Output settings
LUA_SRC = bindings/lua/zstr_module.c
LUA_OUT = zstr.so

BENCH_DIR = benchmarks/c
SDS_URL   = https://raw.githubusercontent.com/antirez/sds/master

all: bundle lua

# Bundle the core C library.
bundle:
	@echo "Bundling $(DIST_HEADER)..."
	python3 $(BUNDLER) $(SRC_CORE) $(DIST_HEADER)

# Build Lua Bindings.
lua: LUA_CURRENT_INC = $(LUA_STD_INC)
lua: LUA_CURRENT_LIB = $(LUA_STD_LIB)
lua: build_msg = "Building Standard Lua 5.4 binding..."
lua: bundle $(LUA_SRC) build_shared

luajit: LUA_CURRENT_INC = $(LUA_JIT_INC)
luajit: LUA_CURRENT_LIB = $(LUA_JIT_LIB)
luajit: build_msg = "Building LuaJIT binding..."
luajit: bundle $(LUA_SRC) build_shared

build_shared:
	@echo $(build_msg)
	gcc -O3 -shared -fPIC -o $(LUA_OUT) $(LUA_SRC) -I. -I$(LUA_CURRENT_INC) -l$(LUA_CURRENT_LIB)

# Helper: Download SDS source only if missing
download_sds:
	@mkdir -p $(BENCH_DIR)
	@if [ ! -f $(BENCH_DIR)/sds.h ]; then \
		echo "Downloading SDS for benchmarks..."; \
		wget -q $(SDS_URL)/sds.h -O $(BENCH_DIR)/sds.h; \
		wget -q $(SDS_URL)/sds.c -O $(BENCH_DIR)/sds.c; \
		wget -q $(SDS_URL)/sdsalloc.h -O $(BENCH_DIR)/sdsalloc.h; \
	fi

# C Benchmark: zstr vs Raw Malloc.
bench_c: bundle
	@echo "=> Compiling C benchmarks (zstr vs Malloc)"
	gcc -O3 -o $(BENCH_DIR)/bench_c $(BENCH_DIR)/main.c -I.
	@echo "Running..."
	./$(BENCH_DIR)/bench_c

# C Benchmark: zstr vs SDS.
bench_sds: bundle download_sds
	@echo "=> Compiling SDS benchmarks"
	gcc -O3 -o $(BENCH_DIR)/bench_sds $(BENCH_DIR)/bench_sds.c $(BENCH_DIR)/sds.c -I. -I$(BENCH_DIR)
	@echo "Running..."
	./$(BENCH_DIR)/bench_sds

# Lua Benchmark: zstr vs Native Lua.
bench_lua: luajit
	@echo "=> Running Lua Benchmarks"
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/general.lua
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/jit.lua
	LUA_CPATH="./?.so;;" luajit benchmarks/lua/bulk.lua

bench: bench_c bench_sds bench_lua


clean:
	rm -f $(LUA_OUT)
	rm -f $(BENCH_DIR)/bench_c $(BENCH_DIR)/bench_sds
	# Optional: cleanup SDS files if you want a fresh start
	# rm -f $(BENCH_DIR)/sds*

init:
	git submodule update --init --recursive

.PHONY: all bundle lua luajit build_shared bench bench_c bench_sds bench_lua download_sds clean init
