-- Test zero-copy TensorView

print("=== Testing TensorView ===")

-- Create view (wraps large C++ array)
local view = Test.createView()
print("View length:", #view)
assert(#view == 10000000, "View length incorrect")

-- Test element access via method
local val = view:get(1)
print("view:get(1) =", val)
assert(math.abs(val - 3.14) < 0.01, "View element access failed")

-- Test element modification via method
view:set(1, 2.71)
assert(math.abs(view:get(1) - 2.71) < 0.01, "View element modification failed")

print("✓ TensorView test PASSED")
