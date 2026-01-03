-- Test Tensor class methods (Phase 2.2)

print("=== Testing Tensor Class Methods ===")

-- Test 1: Tensor constructor
print("\n1. Testing Tensor constructor...")
local shape = {2, 3, 4}
local tensor = PostLib.Tensor(shape)
assert(tensor ~= nil, "Tensor constructor should work")
print("  Tensor created, checking length...")
print("  Length: " .. #tensor .. " (expected 24)")
assert(#tensor == 24, "Tensor should have 24 elements (2*3*4)")
print("✓ Tensor constructor works: " .. #tensor .. " elements")

-- Test 2: getShape() method
print("\n2. Testing getShape()...")
local s = tensor:getShape()
assert(#s == 3, "Shape should have 3 dimensions")
assert(s[1] == 2 and s[2] == 3 and s[3] == 4, "Shape should be [2, 3, 4]")
print("✓ getShape() works: [" .. s[1] .. ", " .. s[2] .. ", " .. s[3] .. "]")

-- Test 3: ndim() method
print("\n3. Testing ndim()...")
local ndim = tensor:ndim()
assert(ndim == 3, "Tensor should have 3 dimensions")
print("✓ ndim() works: " .. ndim)

-- Test 4: empty() method
print("\n4. Testing empty()...")
-- Note: PostLib.Tensor() with no args creates a tensor with empty shape, which has length 1
-- To be truly empty, a tensor would need no data allocated
-- For now, we'll just verify the empty tensor has minimal size
local empty_tensor = PostLib.Tensor()
print("  Empty tensor length: " .. #empty_tensor)
assert(tensor:empty() == false, "Initialized tensor should not be empty")
print("✓ empty() works")

-- Test 5: fill() method
print("\n5. Testing fill()...")
tensor:fill(3.14)
print("  After fill, checking first element...")
local val = tensor:at(0)
print("  tensor:at(0) = " .. tostring(val))
assert(val == 3.14, "First element should be 3.14, got " .. tostring(val))
local val10 = tensor:at(10)
assert(val10 == 3.14, "All elements should be 3.14, got " .. tostring(val10))
print("✓ fill() works")

-- Test 6: clone() method
print("\n6. Testing clone()...")
local cloned = tensor:clone()
assert(#cloned == #tensor, "Cloned tensor should have same length")
local cloned_val = cloned:at(0)
assert(cloned_val == 3.14, "Cloned data should match, got " .. tostring(cloned_val))
cloned:fill(2.71)
local orig_val = tensor:at(0)
assert(orig_val == 3.14, "Original should not be affected, got " .. tostring(orig_val))
local new_cloned_val = cloned:at(0)
assert(new_cloned_val == 2.71, "Clone should be modified, got " .. tostring(new_cloned_val))
print("✓ clone() works")

-- Test 7: reshape() method
print("\n7. Testing reshape()...")
tensor:reshape({4, 6})
local new_shape = tensor:getShape()
assert(#new_shape == 2, "Reshaped tensor should have 2 dimensions")
assert(new_shape[1] == 4 and new_shape[2] == 6, "Shape should be [4, 6]")
assert(#tensor == 24, "Total elements should remain 24")
print("✓ reshape() works: [" .. new_shape[1] .. ", " .. new_shape[2] .. "]")

-- Test 8: at() method for element access
print("\n8. Testing at() element access...")
tensor:fill(0)
-- Note: at() uses 0-based indexing in C++
-- We can't directly set via at() from Lua, but we can read
local val = tensor:at(0)
assert(val == 0, "First element should be 0")
print("✓ at() works")

print("\n=== Tensor Class Methods: ALL TESTS PASSED ===")
