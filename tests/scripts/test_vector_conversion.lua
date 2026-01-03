-- Test std::vector<Box> ↔ Lua table conversion (Phase 2.3)

print("=== Testing Vector<Box> Conversion ===")

-- Test 1: C++ returns vector<Box>, Lua receives table
print("\n1. Testing C++ vector<Box> → Lua table...")
local boxes = PostLib.createTestBoxes()
assert(boxes ~= nil, "Should return boxes table")
assert(type(boxes) == "table", "Should be Lua table, got " .. type(boxes))
print("  Got table with " .. #boxes .. " boxes")
assert(#boxes > 0, "Should have at least one box")
print("✓ C++ vector<Box> converted to Lua table")

-- Test 2: Verify # operator works (length)
print("\n2. Testing # operator on box table...")
local count = #boxes
assert(count >= 3, "Should have at least 3 boxes, got " .. count)
print("✓ # operator works: " .. count .. " boxes")

-- Test 3: Verify ipairs iteration works
print("\n3. Testing ipairs iteration...")
local iterated = 0
for i, box in ipairs(boxes) do
    assert(type(box) == "table" or type(box) == "userdata", "Each element should be table or userdata")
    iterated = iterated + 1
end
assert(iterated == count, "ipairs should iterate all elements")
print("✓ ipairs iteration works: " .. iterated .. " boxes")

-- Test 4: Verify box field access
print("\n4. Testing box field access...")
local first_box = boxes[1]
assert(first_box ~= nil, "First box should exist")

-- Check all required fields exist
local function checkBoxFields(box, index)
    local fields = {"x1", "y1", "x2", "y2", "confidence", "classId"}
    for _, field in ipairs(fields) do
        local value
        if type(box) == "table" then
            value = box[field]
        else
            -- userdata, try as property
            value = box[field]
        end
        assert(value ~= nil, "Box " .. index .. " missing field: " .. field)
    end
end

checkBoxFields(first_box, 1)
print("✓ Box fields accessible")

-- Test 5: Verify field values are reasonable
print("\n5. Testing box field values...")
local function checkBoxValues(box, index)
    local x1, y1, x2, y2, conf, cls
    if type(box) == "table" then
        x1, y1, x2, y2, conf, cls = box.x1, box.y1, box.x2, box.y2, box.confidence, box.classId
    else
        x1, y1, x2, y2, conf, cls = box.x1, box.y1, box.x2, box.y2, box.confidence, box.classId
    end
    
    assert(type(x1) == "number", "x1 should be number")
    assert(type(y1) == "number", "y1 should be number")
    assert(type(x2) == "number", "x2 should be number")
    assert(type(y2) == "number", "y2 should be number")
    assert(type(conf) == "number", "confidence should be number")
    assert(type(cls) == "number", "classId should be number")
    
    assert(x2 > x1, "x2 should be greater than x1")
    assert(y2 > y1, "y2 should be greater than y1")
    assert(conf >= 0 and conf <= 1, "confidence should be in [0, 1]")
    assert(cls >= 0, "classId should be non-negative")
end

checkBoxValues(first_box, 1)
print("✓ Box values are valid")

-- Test 6: Test Lua table → C++ vector conversion
print("\n6. Testing Lua table → C++ vector<Box>...")
local lua_boxes = {
    PostLib.Box(10, 20, 110, 120, 0.9, 0),
    PostLib.Box(200, 200, 300, 300, 0.85, 1),
    PostLib.Box(400, 100, 500, 200, 0.75, 2)
}

-- This function should accept Lua table and process it
local result = PostLib.consumeBoxTable(lua_boxes)
assert(result == true, "consumeBoxTable should return true for valid input")
print("✓ Lua table → C++ vector<Box> conversion works")

-- Test 7: Test empty vector
print("\n7. Testing empty vector...")
local empty_boxes = PostLib.createEmptyBoxes()
assert(type(empty_boxes) == "table", "Should return table even when empty")
assert(#empty_boxes == 0, "Empty vector should have length 0")
print("✓ Empty vector conversion works")

-- Test 8: Test large vector (performance check)
print("\n8. Testing large vector conversion...")
local large_boxes = PostLib.createLargeBoxes(100)
assert(#large_boxes == 100, "Should have exactly 100 boxes")
local sample = large_boxes[50]
checkBoxFields(sample, 50)
print("✓ Large vector conversion works: " .. #large_boxes .. " boxes")

print("\n=== Vector<Box> Conversion: ALL TESTS PASSED ===")
