-- Test Image class methods (Phase 2.2)

print("=== Testing Image Class Methods ===")

-- Test 1: Image constructor
print("\n1. Testing Image constructor...")
local img = CVLib.Image(100, 100, 3)
assert(img ~= nil, "Image constructor should work")
assert(img.width == 100, "Width should be 100")
assert(img.height == 100, "Height should be 100")
assert(img.channels == 3, "Channels should be 3")
print("✓ Image constructor works")

-- Test 2: empty() method
print("\n2. Testing empty() method...")
local empty_img = CVLib.Image()
assert(empty_img:empty() == true, "Default image should be empty")
assert(img:empty() == false, "100x100 image should not be empty")
print("✓ empty() works")

-- Test 3: fill() method
print("\n3. Testing fill() method...")
img:fill(128)
assert(img:at(0, 0, 0) == 128, "Pixel should be 128 after fill")
assert(img:at(50, 50, 1) == 128, "All pixels should be 128")
print("✓ fill() works")

-- Test 4: clone() method
print("\n4. Testing clone() method...")
local cloned = img:clone()
assert(cloned.width == img.width, "Cloned width should match")
assert(cloned.height == img.height, "Cloned height should match")
assert(cloned:at(0, 0, 0) == 128, "Cloned pixel should match")
cloned:fill(255)
assert(img:at(0, 0, 0) == 128, "Original should not be affected")
assert(cloned:at(0, 0, 0) == 255, "Clone should be modified")
print("✓ clone() works")

-- Test 5: crop function
print("\n5. Testing crop()...")
local original = CVLib.imread("test.jpg")
local cropped = CVLib.crop(original, 100, 100, 200, 200)
assert(cropped.width == 200, "Cropped width should be 200")
assert(cropped.height == 200, "Cropped height should be 200")
print("✓ crop() works: " .. cropped.width .. "x" .. cropped.height)

-- Test 6: resize function
print("\n6. Testing resize()...")
local resized = CVLib.resize(original, 320, 240)
assert(resized.width == 320, "Resized width should be 320")
assert(resized.height == 240, "Resized height should be 240")
print("✓ resize() works: " .. resized.width .. "x" .. resized.height)

-- Test 7: flipHorizontal function
print("\n7. Testing flipHorizontal()...")
local flipped_h = CVLib.flipHorizontal(original)
assert(flipped_h.width == original.width, "Width should be preserved")
assert(flipped_h.height == original.height, "Height should be preserved")
print("✓ flipHorizontal() works")

-- Test 8: flipVertical function
print("\n8. Testing flipVertical()...")
local flipped_v = CVLib.flipVertical(original)
assert(flipped_v.width == original.width, "Width should be preserved")
assert(flipped_v.height == original.height, "Height should be preserved")
print("✓ flipVertical() works")

print("\n=== Image Class Methods: ALL TESTS PASSED ===")
