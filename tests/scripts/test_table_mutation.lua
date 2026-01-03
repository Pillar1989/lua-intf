-- Test in-place Lua table mutation

print("=== Testing table mutation ===")

local t = {10, 20, 30}
local addr_before = tostring(t)
print("Table before:", table.concat(t, ", "))
print("Address before:", addr_before)

-- Call C++ function that modifies table
Test.modifyTable(t)

local addr_after = tostring(t)
print("Table after:", table.concat(t, ", "))
print("Address after:", addr_after)

-- Verify table identity preserved
assert(addr_before == addr_after, "Table was copied, not modified in-place")

-- Verify modification result
assert(#t == 2, "Table size wrong: expected 2, got " .. #t)
assert(t[1] == 10 and t[2] == 30, "Table values wrong after modification")

print("✓ Table mutation test PASSED")
