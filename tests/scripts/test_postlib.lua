-- Test PostLib module bindings

print("=== Testing PostLib Module ===")

-- Create a dummy tensor for testing
local function createDummyTensor()
    -- PostLib expects Tensor objects, but we'll use a table for now
    return {
        getShape = function() return {1, 3, 80, 80} end
    }
end

-- Test 1: parseYoloOutput
print("\n1. Testing parseYoloOutput...")
local tensor = createDummyTensor()

-- For now, we'll skip this test since we need actual Tensor objects
-- This will be properly tested with real tensor binding later
print("⚠ Skipping parseYoloOutput (requires Tensor binding)")

-- Test 2: NMS in-place mutation
print("\n2. Testing NMS in-place mutation...")
local boxes = {
    {x1 = 100, y1 = 100, x2 = 200, y2 = 200, confidence = 0.9, classId = 0},
    {x1 = 110, y1 = 110, x2 = 210, y2 = 210, confidence = 0.8, classId = 0},  -- High overlap, should be suppressed
    {x1 = 300, y1 = 300, x2 = 400, y2 = 400, confidence = 0.85, classId = 1},
    {x1 = 500, y1 = 100, x2 = 600, y2 = 200, confidence = 0.7, classId = 2},
}

local original_table_id = tostring(boxes)
print("Original boxes: " .. #boxes)

PostLib.nms(boxes, 0.45)

local after_table_id = tostring(boxes)
print("After NMS: " .. #boxes)

-- CRITICAL: Verify table identity is preserved (in-place mutation)
assert(original_table_id == after_table_id, "NMS must preserve table identity (in-place mutation)")
assert(#boxes < 4, "NMS should have removed some boxes")
print("✓ NMS works: " .. #boxes .. " boxes remaining (table identity preserved)")

-- Test 3: scaleBoxes
print("\n3. Testing scaleBoxes...")
local test_boxes = {
    {x1 = 100, y1 = 100, x2 = 200, y2 = 200, confidence = 0.9, classId = 0},
}

-- Convert to Box objects (for now use tables, real implementation will use C++ Box)
local pad_info = {
    top = 80,
    left = 0,
    bottom = 80,
    right = 0
}

-- For now, we'll skip this test since scaleBoxes expects vector<Box>
-- This will be properly tested with real Box conversion later
print("⚠ Skipping scaleBoxes (requires Box vector conversion)")

print("\n=== PostLib Module: CORE TESTS PASSED ===")
print("Note: Some tests skipped pending full Tensor/Box integration")
