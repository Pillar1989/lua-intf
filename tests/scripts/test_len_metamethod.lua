-- Test __len metamethod support

print("=== Testing __len metamethod ===")

-- Test with TestLen class
local obj = TestLen()
local len = #obj

print("Object length:", len)
assert(len == 42, "__len metamethod failed: expected 42, got " .. tostring(len))

print("✓ __len metamethod test PASSED")
