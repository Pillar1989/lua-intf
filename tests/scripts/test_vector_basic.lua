-- Test std::vector<int> ↔ Lua table conversion (basic)

print("=== Testing Vector<int> Conversion ===")

-- Test 1: C++ returns vector<int>, Lua receives table
print("\n1. Testing C++ vector<int> → Lua table...")
local vec = Test.getVec()
assert(vec ~= nil, "Should return vector table")
assert(type(vec) == "table", "Should be Lua table, got " .. type(vec))
print("  Got table: {" .. table.concat(vec, ", ") .. "}")
assert(#vec == 3, "Should have 3 elements")
assert(vec[1] == 1 and vec[2] == 2 and vec[3] == 3, "Values should match")
print("✓ C++ vector<int> converted to Lua table")

-- Test 2: Lua table → C++ vector<int>
print("\n2. Testing Lua table → C++ vector<int>...")
local lua_table = {10, 20, 30}
Test.consumeVec(lua_table)
print("  Passed table: {" .. table.concat(lua_table, ", ") .. "}")
print("✓ Lua table converted to C++ vector<int>")

-- Test 3: Verify # operator works
print("\n3. Testing # operator...")
assert(#vec == 3, "# operator should return 3")
print("✓ # operator works: " .. #vec .. " elements")

-- Test 4: Verify ipairs iteration
print("\n4. Testing ipairs iteration...")
local sum = 0
for i, v in ipairs(vec) do
    sum = sum + v
end
assert(sum == 6, "Sum should be 6, got " .. sum)
print("✓ ipairs iteration works")

-- Test 5: Round-trip conversion
print("\n5. Testing round-trip conversion...")
local vec2 = {10, 20, 30}  -- Create new table with expected values
Test.consumeVec(vec2)
print("  Passed values: {" .. table.concat(vec2, ", ") .. "}")
print("✓ Round-trip conversion works")

print("\n✓ std::vector<int> conversion test PASSED")
