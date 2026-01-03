-- Test real preprocess.lua script

print("=== Testing scripts/preprocess.lua ===")

-- Load the preprocess script
dofile("scripts/preprocess.lua")

-- Test preprocessing
local tensor, pad_info = preprocess("test.jpg", nil)

print("\n1. Tensor output:")
print("  Length: " .. #tensor)
assert(#tensor == 640 * 640 * 3, "Tensor should have 640*640*3 elements")
print("  ✓ Tensor has correct size")

print("\n2. Pad info:")
print("  top: " .. pad_info.top)
print("  left: " .. pad_info.left)
print("  bottom: " .. pad_info.bottom)
print("  right: " .. pad_info.right)
assert(pad_info.top ~= nil, "pad_info should have top field")
assert(pad_info.left ~= nil, "pad_info should have left field")
print("  ✓ Pad info has correct structure")

print("\n=== scripts/preprocess.lua: PASSED ===")
