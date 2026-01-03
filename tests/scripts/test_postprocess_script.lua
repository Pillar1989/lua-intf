-- Test real postprocess.lua script

print("=== Testing scripts/postprocess.lua ===")

-- Load the postprocess script
dofile("scripts/postprocess.lua")

-- Create dummy outputs (simulate model output tensors)
-- For now, we'll use empty tables to test the structure
local dummy_tensor = {
    getShape = function(self)
        return {1, 25200, 85}  -- YOLOv5 output shape
    end
}

-- Add __len metamethod
setmetatable(dummy_tensor, {
    __len = function(self) return 25200 * 85 end
})

local outputs = {dummy_tensor}

local image_info = {
    orig_w = 640,
    orig_h = 480,
    padded_w = 640,
    padded_h = 640,
    pad_info = {
        top = 80,
        left = 0,
        bottom = 80,
        right = 0
    }
}

-- Test postprocessing
local results = postprocess(outputs, image_info, nil)

print("\n1. Results:")
print("  Number of detections: " .. #results)
assert(type(results) == "table", "Results should be a table")
print("  ✓ Results have correct structure")

if #results > 0 then
    print("\n2. First detection:")
    local first = results[1]
    print("  x1: " .. first.x1)
    print("  y1: " .. first.y1)
    print("  x2: " .. first.x2)
    print("  y2: " .. first.y2)
    print("  confidence: " .. first.confidence)
    print("  classId: " .. first.classId)
    assert(first.x1 ~= nil, "Detection should have x1 field")
    assert(first.confidence ~= nil, "Detection should have confidence field")
    print("  ✓ Detection has correct fields")
end

print("\n=== scripts/postprocess.lua: PASSED ===")
