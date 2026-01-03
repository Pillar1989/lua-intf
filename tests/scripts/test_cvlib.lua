-- Test CVLib module bindings

print("=== Testing CVLib Module ===")

-- Test 1: imread
print("\n1. Testing imread...")
local img = CVLib.imread("test.jpg")
assert(img ~= nil, "imread should return an image")
assert(img.width == 640, "Image width should be 640")
assert(img.height == 480, "Image height should be 480")
print("✓ imread works: " .. img.width .. "x" .. img.height)

-- Test 2: bgr2rgb
print("\n2. Testing bgr2rgb...")
local rgb_img = CVLib.bgr2rgb(img)
assert(rgb_img ~= nil, "bgr2rgb should return an image")
assert(rgb_img.width == img.width, "Width should be preserved")
assert(rgb_img.height == img.height, "Height should be preserved")
print("✓ bgr2rgb works: " .. rgb_img.width .. "x" .. rgb_img.height)

-- Test 3: letterbox
print("\n3. Testing letterbox...")
local result = CVLib.letterbox(rgb_img, 640, 640)
assert(result ~= nil, "letterbox should return a result")
assert(result.image ~= nil, "result should have image field")
assert(result.pad ~= nil, "result should have pad field")
assert(result.image.width == 640, "Padded width should be 640")
assert(result.image.height == 640, "Padded height should be 640")
assert(result.pad.top ~= nil, "pad should have top field")
assert(result.pad.left ~= nil, "pad should have left field")
print("✓ letterbox works: padded to 640x640, pad={top=" .. result.pad.top .. ", left=" .. result.pad.left .. "}")

-- Test 4: normalize
print("\n4. Testing normalize...")
local tensor_data = CVLib.normalize(result.image)
assert(tensor_data ~= nil, "normalize should return data")
assert(#tensor_data == 640 * 640 * 3, "Normalized data should have correct size")
print("✓ normalize works: " .. #tensor_data .. " elements")

-- Test 5: hwc2chw
print("\n5. Testing hwc2chw...")
local chw_data = CVLib.hwc2chw(result.image)
assert(chw_data ~= nil, "hwc2chw should return data")
assert(#chw_data == 640 * 640 * 3, "CHW data should have correct size")
print("✓ hwc2chw works: " .. #chw_data .. " elements")

print("\n=== CVLib Module: ALL TESTS PASSED ===")
