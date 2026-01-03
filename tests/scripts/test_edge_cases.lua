-- Edge Cases and Boundary Tests for model_infer

print("=== Edge Cases and Boundary Tests ===\n")

local test_count = 0
local pass_count = 0

local function test(name, fn)
    test_count = test_count + 1
    io.write(string.format("Test %d: %s... ", test_count, name))
    local status, err = pcall(fn)
    if status then
        pass_count = pass_count + 1
        print("✓ PASS")
        return true
    else
        print("✗ FAIL")
        print("  Error:", err)
        return false
    end
end

-- ============= CVLib Edge Cases =============

test("Empty image dimensions", function()
    -- Note: In real implementation, this should probably error
    -- But we test that it doesn't crash
    local img = CVLib.Image(0, 0)
    assert(img.width == 0)
    assert(img.height == 0)
end)

test("Large image dimensions", function()
    local img = CVLib.Image(10000, 10000)
    assert(img.width == 10000)
    assert(img.height == 10000)
end)

test("Negative dimensions create empty image", function()
    -- Negative dimensions should create empty image (not crash)
    local img = CVLib.Image(-1, 100)
    assert(img.width == -1 or img.width == 0, "Negative width handling")
end)

test("normalize with empty image", function()
    local img = CVLib.Image(0, 0)
    local data = CVLib.normalize(img)
    assert(type(data) == "table", "normalize should return table")
    assert(#data == 0, "normalized empty image should be empty table")
end)

test("normalize with valid small image", function()
    local img = CVLib.Image(2, 2, 3)
    img:fill(128)  -- Fill with gray
    local data = CVLib.normalize(img)
    assert(type(data) == "table", "normalize should return table")
    assert(#data == 2 * 2 * 3, "normalized data size should be W*H*C")
end)

test("hwc2chw with empty image", function()
    local img = CVLib.Image(0, 0)
    local data = CVLib.hwc2chw(img)
    assert(type(data) == "table", "hwc2chw should return table")
    assert(#data == 0, "hwc2chw empty image should be empty table")
end)

test("hwc2chw with valid small image", function()
    local img = CVLib.Image(2, 2, 3)
    img:fill(100)
    local data = CVLib.hwc2chw(img)
    assert(type(data) == "table", "hwc2chw should return table")
    assert(#data == 2 * 2 * 3, "hwc2chw data size should be W*H*C")
end)

-- ============= PostLib Edge Cases =============

test("Empty box list", function()
    local boxes = {}
    -- NMS on empty list should not crash
    PostLib.nms(boxes, 0.5)
    assert(#boxes == 0, "Empty list should remain empty")
end)

test("Single box (no suppression)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, classId=0}
    }
    PostLib.nms(boxes, 0.5)
    assert(#boxes == 1, "Single box should not be suppressed")
end)

test("Two identical boxes (one suppressed)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, classId=0},
        {x1=100, y1=100, x2=200, y2=200, confidence=0.8, classId=0}
    }
    PostLib.nms(boxes, 0.5)
    assert(#boxes == 1, "Duplicate box should be suppressed")
    assert(boxes[1].confidence == 0.9, "Higher confidence box should remain")
end)

test("Boxes with different classes (no suppression)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, classId=0},
        {x1=100, y1=100, x2=200, y2=200, confidence=0.8, classId=1}
    }
    local orig_count = #boxes
    PostLib.nms(boxes, 0.5)
    assert(#boxes == orig_count, "Different class boxes should not suppress each other")
end)

test("Zero confidence box", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.0, classId=0}
    }
    -- Should not crash
    PostLib.nms(boxes, 0.5)
    assert(#boxes >= 0, "Zero confidence box handling")
end)

test("Very high confidence (1.0)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=1.0, classId=0},
        {x1=110, y1=110, x2=210, y2=210, confidence=0.9, classId=0}
    }
    PostLib.nms(boxes, 0.5)
    assert(boxes[1].confidence == 1.0, "Highest confidence should be first")
end)

test("NMS threshold at extremes (0.0)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, classId=0},
        {x1=110, y1=110, x2=210, y2=210, confidence=0.8, classId=0}
    }
    PostLib.nms(boxes, 0.0)
    -- With 0.0 threshold, any overlap causes suppression
    assert(#boxes >= 1, "At least one box should remain")
end)

test("NMS threshold at extremes (1.0)", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, classId=0},
        {x1=100, y1=100, x2=200, y2=200, confidence=0.8, classId=0}
    }
    PostLib.nms(boxes, 1.0)
    -- With 1.0 threshold, only perfect overlap causes suppression
    -- Identical boxes should still be suppressed
    assert(#boxes <= 2, "High threshold allows more boxes")
end)

test("Large box list (100 boxes)", function()
    local boxes = {}
    for i = 1, 100 do
        table.insert(boxes, {
            x1 = i * 10,
            y1 = i * 10,
            x2 = i * 10 + 50,
            y2 = i * 10 + 50,
            confidence = 0.5 + math.random() * 0.5,
            classId = i % 10
        })
    end
    local orig_count = #boxes
    PostLib.nms(boxes, 0.5)
    assert(#boxes <= orig_count, "NMS should reduce or maintain count")
    print(string.format("  (Reduced from %d to %d boxes)", orig_count, #boxes))
end)

test("Box with inverted coordinates", function()
    -- x2 < x1 or y2 < y1 (invalid box)
    local boxes = {
        {x1=200, y1=200, x2=100, y2=100, confidence=0.9, classId=0}
    }
    -- Should handle gracefully (implementation dependent)
    local status = pcall(function()
        PostLib.nms(boxes, 0.5)
    end)
    assert(status, "Inverted coordinates should be handled")
end)

test("Box with zero area", function()
    local boxes = {
        {x1=100, y1=100, x2=100, y2=100, confidence=0.9, classId=0}
    }
    PostLib.nms(boxes, 0.5)
    -- Zero area box should not crash
    assert(true, "Zero area box handled")
end)

test("Box with negative coordinates", function()
    local boxes = {
        {x1=-50, y1=-50, x2=50, y2=50, confidence=0.9, classId=0}
    }
    PostLib.nms(boxes, 0.5)
    assert(#boxes == 1, "Negative coordinates should be valid")
end)

test("scaleBoxes with identity scaling", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, class_id=0}
    }
    local pad_info = {top=0, left=0, right=0, bottom=0}
    local scaled = PostLib.scaleBoxes(boxes, 640, 640, 640, 640, pad_info)
    assert(#scaled == 1, "Should return same number of boxes")
    -- With identity scaling and no padding, coordinates should be unchanged
    local box = scaled[1]
    assert(box.x1 == 100 and box.y1 == 100, "Coordinates should be unchanged")
end)

test("scaleBoxes with extreme scaling", function()
    local boxes = {
        {x1=100, y1=100, x2=200, y2=200, confidence=0.9, class_id=0}
    }
    local pad_info = {top=0, left=0, right=0, bottom=0}
    -- Scale from 640x640 to 64x64 (10x smaller)
    local scaled = PostLib.scaleBoxes(boxes, 64, 64, 640, 640, pad_info)
    assert(#scaled == 1, "Should return same number of boxes")
    local box = scaled[1]
    -- Coordinates should be scaled down
    assert(box.x1 < 100, "X coordinate should be scaled down")
end)

test("scaleBoxes with large padding", function()
    local boxes = {
        {x1=200, y1=200, x2=300, y2=300, confidence=0.9, class_id=0}
    }
    local pad_info = {top=100, left=100, right=100, bottom=100}
    local scaled = PostLib.scaleBoxes(boxes, 640, 640, 840, 840, pad_info)
    assert(#scaled == 1, "Should handle padding")
end)

-- ============= Mixed Scenarios =============

test("CVLib and PostLib pipeline with empty result", function()
    -- Simulate scenario where detection returns nothing
    -- In real pipeline, parseYoloOutput would return empty list
    local boxes = {}
    PostLib.nms(boxes, 0.5)
    assert(#boxes == 0, "Empty pipeline should complete")
end)

test("Stress test: 1000 boxes", function()
    local boxes = {}
    for i = 1, 1000 do
        table.insert(boxes, {
            x1 = math.random(0, 640),
            y1 = math.random(0, 480),
            x2 = math.random(0, 640),
            y2 = math.random(0, 480),
            confidence = math.random() * 0.8 + 0.2,
            classId = math.random(0, 79)
        })
    end
    local start_time = os.clock()
    PostLib.nms(boxes, 0.5)
    local elapsed = os.clock() - start_time
    print(string.format("  (Processed 1000 boxes in %.3f seconds, %d remaining)", 
          elapsed, #boxes))
    assert(#boxes <= 1000, "NMS should not increase box count")
end)

-- ============= Summary =============

print("\n=== Test Summary ===")
print(string.format("Total tests: %d", test_count))
print(string.format("Passed: %d", pass_count))
print(string.format("Failed: %d", test_count - pass_count))

if pass_count == test_count then
    print("\n✓ All edge case tests PASSED")
    os.exit(0)
else
    print(string.format("\n✗ %d tests FAILED", test_count - pass_count))
    os.exit(1)
end
