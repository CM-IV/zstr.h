
local zstr = require("zstr")

print("=> Processing pipeline\n")

-- INGEST!
-- Create a buffer and simulate reading a messy log file.
local pipeline = zstr.new()
pipeline:reserve(1024) -- Optional: Pre-allocate if we know rough size.

print("-> [1] Ingesting simulated log data...")
pipeline:append("[INFO]   User:Alice   ip=192.168.1.50   pass=secret123   \n")
pipeline:append("[WARN]   User:Bob     ip=10.0.0.2       pass=qwerty      \n")
pipeline:append("[ERR]    User:Admin   ip=127.0.0.1      pass=admin       \n")

print(string.format("   Buffer: %d bytes (Capacity: %d)", #pipeline, pipeline:capacity()))


-- CLEANUP! (in-place)
-- Trim whitespace and sanitize sensitive info directly in the buffer.
print("\n-> [2] Sanitizing data...")

-- Transformation: Remove leading/trailing whitespace
pipeline:trim() 

-- Sanitization: Redact passwords.
-- This modifies the memory in-place. No new string objects created.
pipeline:replace("pass=secret123", "pass=******")
pipeline:replace("pass=qwerty",    "pass=******")
pipeline:replace("pass=admin",     "pass=******")

-- Normalization: Convert to uppercase for standardizing tags.
pipeline:upper()

print("   Sanitization complete.")


-- ANALYSIS!
-- Check for errors and validate encoding.
print("\n-> [3] Analyzing...")

if pipeline:contains("[ERR]") then
    print("   [ALERT] Error log detected!")
end

if pipeline:is_valid_utf8() then
    print("   [OK] Data is valid UTF-8.")
else
    print("   [FAIL] Corrupt encoding detected.")
end

print("   Total Runes (Chars): " .. pipeline:rune_count())


-- EXPORT!
-- Split the buffer back into Lua strings for line-by-line processing.
print("\n-> [4] Exporting lines...")

local lines = pipeline:split("\n")

for i, line in ipairs(lines) do
    -- We can convert the line back to a temporary zstr to trim it individually
    -- or just use it as a Lua string, whatever.
    print(string.format("   Line %d: %s", i, line))
end


-- CLEANUP!
pipeline:clear()
print("\n-> Pipeline cleared. Size: " .. #pipeline)
