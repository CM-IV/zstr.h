local zstr = require("zstr")

-- Configuration
local ITERATIONS = 100000  -- How many appends to perform
local APPEND_STR = "1234567890" -- 10 bytes

print(string.format("** Benchmark: %d iterations **", ITERATIONS))
print(string.format("Appending payload size: %d bytes\n", #APPEND_STR))

-- Helper to format numbers
local function format_num(n)
    return tostring(math.floor(n * 1000) / 1000)
end

---------------------------------------------------------
-- TEST 1: Native Lua String Concatenation
---------------------------------------------------------
print("-> Testing Native Lua String (..)")

collectgarbage() -- clear memory before starting
local start_mem_lua = collectgarbage("count")
local start_time_lua = os.clock()

local lua_str = ""
for i = 1, ITERATIONS do
    lua_str = lua_str .. APPEND_STR
end

local end_time_lua = os.clock()
local end_mem_lua = collectgarbage("count")
local lua_duration = end_time_lua - start_time_lua
local lua_garbage = end_mem_lua - start_mem_lua

print("   Time:    " .. format_num(lua_duration) .. "s")
print("   Memory Delta (Garbage): " .. format_num(lua_garbage) .. " KB")
print("   Final Length: " .. #lua_str)


---------------------------------------------------------
-- TEST 2: zstr Mutable Buffer
---------------------------------------------------------
print("\n-> Testing zstr Mutable Buffer (:append)")

collectgarbage()
local start_mem_z = collectgarbage("count")
local start_time_z = os.clock()

local z_buf = zstr.new()
-- Optional: Reserve capacity if known (fair comparison usually doesn't, but zstr supports it)
-- z_buf:reserve(ITERATIONS * #APPEND_STR) 

for i = 1, ITERATIONS do
    z_buf:append(APPEND_STR)
end

local end_time_z = os.clock()
local end_mem_z = collectgarbage("count")
local z_duration = end_time_z - start_time_z
local z_garbage = end_mem_z - start_mem_z

print("   Time:    " .. format_num(z_duration) .. "s")
print("   Memory Delta (Garbage): " .. format_num(z_garbage) .. " KB")
print("   Final Length: " .. #z_buf)

---------------------------------------------------------
-- RESULTS
---------------------------------------------------------
print("\n** Results **")
if z_duration < lua_duration then
    local speedup = lua_duration / z_duration
    print(string.format("zstr was %.2fx FASTER than Lua.", speedup))
else
    local slowdown = z_duration / lua_duration
    print(string.format("zstr was %.2fx SLOWER than Lua.", slowdown))
end

if z_garbage < lua_garbage then
    print("zstr generated LESS garbage (better for GC).")
else
    print("zstr generated MORE garbage.")
end
