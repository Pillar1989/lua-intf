-- Test nested container conversion

print("=== Testing nested containers ===")

-- Test C++ -> Lua
local nested = Test.createNested()
print("Nested array size:", #nested)
assert(#nested == 3, "Outer array size wrong")

-- Check first inner array
assert(#nested[1] == 3, "Inner array 1 size wrong")
assert(nested[1][1] == 1 and nested[1][2] == 2 and nested[1][3] == 3, "Inner array 1 values wrong")

-- Check second inner array
assert(#nested[2] == 2, "Inner array 2 size wrong")
assert(nested[2][1] == 4 and nested[2][2] == 5, "Inner array 2 values wrong")

-- Check third inner array
assert(#nested[3] == 4, "Inner array 3 size wrong")

-- Test iteration
for i, inner in ipairs(nested) do
    print("  nested[" .. i .. "] = {" .. table.concat(inner, ", ") .. "}")
end

-- Test Lua -> C++
local lua_nested = {{10, 11, 12}, {20, 21}}
Test.consumeNested(lua_nested)  -- Should not throw

print("✓ Nested container test PASSED")
