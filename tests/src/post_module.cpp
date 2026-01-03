#include "post_types.h"
#include "cv_types.h"
#include "LuaIntf.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace LuaIntf;
using namespace PostLib;
using namespace CVLib;

namespace PostLib {

/**
 * Parse YOLO output tensor into boxes.
 * Accepts either Tensor object or Lua table with getShape method.
 * Returns Lua table of boxes.
 * Stub implementation - generates dummy boxes for testing.
 */
LuaRef parseYoloOutput(LuaRef tensor_ref) {
    lua_State* L = tensor_ref.state();
    
    // TODO: Real YOLO parsing logic
    // For now, generate some dummy boxes
    
    // Try to get shape (works for both Tensor objects and tables with getShape)
    std::vector<int> shape;
    if (tensor_ref.type() == LuaTypeID::USERDATA) {
        // Real Tensor object
        Tensor* tensor = static_cast<Tensor*>(tensor_ref.toPtr());
        if (tensor) {
            shape = tensor->getShapeCpp();
        }
    } else if (tensor_ref.isTable()) {
        // Lua table with getShape method
        LuaRef getShape = tensor_ref["getShape"];
        if (getShape.isFunction()) {
            LuaRef shape_ref = getShape.call<LuaRef>(tensor_ref);
            for (auto& kv : shape_ref) {
                shape.push_back(kv.value<int>());
            }
        }
    }
    
    // Create test boxes and return as Lua table
    LuaRef boxes = LuaRef::createTable(L);
    
    std::vector<Box> test_boxes = {
        Box(100, 100, 200, 200, 0.85f, 0),
        Box(150, 150, 250, 250, 0.75f, 0),  // Overlaps with first
        Box(300, 300, 400, 400, 0.90f, 1),
        Box(500, 100, 600, 200, 0.65f, 2)
    };
    
    for (size_t i = 0; i < test_boxes.size(); i++) {
        LuaRef box_table = LuaRef::createTable(L);
        box_table.set("x1", test_boxes[i].x1);
        box_table.set("y1", test_boxes[i].y1);
        box_table.set("x2", test_boxes[i].x2);
        box_table.set("y2", test_boxes[i].y2);
        box_table.set("confidence", test_boxes[i].confidence);
        box_table.set("class_id", test_boxes[i].class_id);
        boxes[i + 1] = box_table;  // Lua 1-based indexing
    }
    
    return boxes;
}

/**
 * Phase 2.3 Test Functions for vector<Box> conversion
 */

// Test 1: Return a vector of Box objects as Lua table
// Note: Manual conversion to LuaRef table because lua-intf auto-conversion may not handle Box correctly
LuaRef createTestBoxes(lua_State* L) {
    std::vector<Box> boxes;
    boxes.push_back(Box(100, 100, 200, 200, 0.85f, 0));
    boxes.push_back(Box(150, 150, 250, 250, 0.75f, 0));
    boxes.push_back(Box(300, 300, 400, 400, 0.90f, 1));
    boxes.push_back(Box(500, 100, 600, 200, 0.65f, 2));
    
    // Convert to Lua table manually
    LuaRef result = LuaRef::createTable(L);
    for (size_t i = 0; i < boxes.size(); i++) {
        // Push Box object and create LuaRef from stack top
        Lua::push(L, boxes[i]);
        result[i + 1] = LuaRef(L, -1);
        lua_pop(L, 1);
    }
    
    return result;
}

// Test 2: Consume a Lua table of boxes and validate structure
bool consumeBoxTable(LuaRef boxes_table) {
    // Count boxes in table
    size_t count = 0;
    for (auto& _ : boxes_table) {
        (void)_;  // Unused
        count++;
    }
    
    if (count != 3) return false;
    
    // Validate first box
    LuaRef first_box = boxes_table[1];
    
    // Try to get as Box userdata
    if (first_box.type() == LuaTypeID::USERDATA) {
        Box* box_ptr = static_cast<Box*>(first_box.toPtr());
        if (!box_ptr) return false;
        
        // Validate values
        if (box_ptr->x1 != 10 || box_ptr->y1 != 20 || 
            box_ptr->x2 != 110 || box_ptr->y2 != 120 ||
            std::abs(box_ptr->confidence - 0.9f) > 0.01f ||
            box_ptr->class_id != 0) {
            return false;
        }
    } else {
        return false;
    }
    
    return true;
}

// Test 3: Return empty vector
LuaRef createEmptyBoxes(lua_State* L) {
    return LuaRef::createTable(L);
}

// Test 4: Return large vector for performance testing
LuaRef createLargeBoxes(lua_State* L, int count) {
    std::vector<Box> boxes;
    for (int i = 0; i < count; i++) {
        float x = static_cast<float>(i * 10);
        boxes.push_back(Box(x, x, x + 50, x + 50, 0.8f, i % 10));
    }
    
    // Convert to Lua table manually
    LuaRef result = LuaRef::createTable(L);
    for (size_t i = 0; i < boxes.size(); i++) {
        Lua::push(L, boxes[i]);
        result[i + 1] = LuaRef(L, -1);
        lua_pop(L, 1);
    }
    
    return result;
}

/**
 * Apply Non-Maximum Suppression (NMS) in-place.
 * Modifies the input table by removing suppressed boxes.
 * Returns nothing (testing void + LuaRef + double combination).
 */
void nms(LuaRef boxes_table, double iou_threshold) {
    float threshold = static_cast<float>(iou_threshold);
    
    // Extract all boxes from Lua table
    std::vector<std::pair<int, Box*>> indexed_boxes;
    
    for (auto& kv : boxes_table) {
        int idx = kv.key<int>();
        LuaRef box_ref = kv.value<LuaRef>();
        
        // Extract box fields from Lua table
        float x1 = box_ref.get<float>("x1");
        float y1 = box_ref.get<float>("y1");
        float x2 = box_ref.get<float>("x2");
        float y2 = box_ref.get<float>("y2");
        float confidence = box_ref.get<float>("confidence");
        
        // Support both "class_id" and "classId" for compatibility
        int class_id = box_ref.has("class_id") ? 
            box_ref.get<int>("class_id") : 
            box_ref.get<int>("classId");
        
        Box* box = new Box(x1, y1, x2, y2, confidence, class_id);
        indexed_boxes.push_back({idx, box});
    }
    
    // Sort by confidence descending
    std::sort(indexed_boxes.begin(), indexed_boxes.end(),
        [](const auto& a, const auto& b) {
            return a.second->confidence > b.second->confidence;
        });
    
    // NMS algorithm: mark suppressed boxes
    std::vector<bool> suppressed(indexed_boxes.size(), false);
    
    for (size_t i = 0; i < indexed_boxes.size(); i++) {
        if (suppressed[i]) continue;
        
        Box* box_i = indexed_boxes[i].second;
        
        for (size_t j = i + 1; j < indexed_boxes.size(); j++) {
            if (suppressed[j]) continue;
            
            Box* box_j = indexed_boxes[j].second;
            
            // Only suppress boxes of same class
            if (box_i->class_id == box_j->class_id) {
                float iou_val = box_i->iou(*box_j);
                if (iou_val > threshold) {
                    suppressed[j] = true;
                }
            }
        }
    }
    
    // Remove suppressed boxes from Lua table (in reverse order to preserve indices)
    int removed_count = 0;
    for (int i = indexed_boxes.size() - 1; i >= 0; i--) {
        if (suppressed[i]) {
            boxes_table.removeAt(indexed_boxes[i].first);
            removed_count++;
        }
        delete indexed_boxes[i].second;
    }
    
    // Compact the table to remove holes
    boxes_table.compact();
}

/**
 * Scale bounding boxes from model input size to original image size.
 * Accounts for letterbox padding.
 * Accepts and returns Lua tables (not C++ vectors).
 */
LuaRef scaleBoxes(
    LuaRef boxes_table,
    int orig_w, int orig_h,
    int padded_w, int padded_h,
    LuaRef pad_info)
{
    lua_State* L = boxes_table.state();
    
    int pad_left = pad_info.get<int>("left");
    int pad_top = pad_info.get<int>("top");
    int pad_right = pad_info.get<int>("right");
    int pad_bottom = pad_info.get<int>("bottom");
    
    float scale_x = static_cast<float>(orig_w) / (padded_w - pad_left - pad_right);
    float scale_y = static_cast<float>(orig_h) / (padded_h - pad_top - pad_bottom);
    
    LuaRef scaled_boxes = LuaRef::createTable(L);
    
    int idx = 1;
    for (auto& kv : boxes_table) {
        LuaRef box = kv.value<LuaRef>();
        
        float x1 = box.get<float>("x1");
        float y1 = box.get<float>("y1");
        float x2 = box.get<float>("x2");
        float y2 = box.get<float>("y2");
        float confidence = box.get<float>("confidence");
        int class_id = box.get<int>("class_id");
        
        // Remove padding offset and scale
        float scaled_x1 = (x1 - pad_left) * scale_x;
        float scaled_y1 = (y1 - pad_top) * scale_y;
        float scaled_x2 = (x2 - pad_left) * scale_x;
        float scaled_y2 = (y2 - pad_top) * scale_y;
        
        // Clamp to image bounds
        scaled_x1 = std::max(0.0f, std::min(scaled_x1, static_cast<float>(orig_w)));
        scaled_y1 = std::max(0.0f, std::min(scaled_y1, static_cast<float>(orig_h)));
        scaled_x2 = std::max(0.0f, std::min(scaled_x2, static_cast<float>(orig_w)));
        scaled_y2 = std::max(0.0f, std::min(scaled_y2, static_cast<float>(orig_h)));
        
        LuaRef scaled_box = LuaRef::createTable(L);
        scaled_box.set("x1", scaled_x1);
        scaled_box.set("y1", scaled_y1);
        scaled_box.set("x2", scaled_x2);
        scaled_box.set("y2", scaled_y2);
        scaled_box.set("confidence", confidence);
        scaled_box.set("class_id", class_id);
        
        scaled_boxes[idx++] = scaled_box;
    }
    
    return scaled_boxes;
}

/**
 * Get Tensor shape as Lua table.
 */
int tensorGetShape(lua_State* L) {
    Tensor* tensor = Lua::get<Tensor*>(L, 1);
    if (!tensor) {
        return luaL_error(L, "getShape: expected Tensor");
    }
    
    std::vector<int> shape = tensor->getShapeCpp();
    LuaRef result = LuaRef::createTable(L);
    for (size_t i = 0; i < shape.size(); i++) {
        result.set(i + 1, shape[i]);
    }
    
    result.pushToStack();
    return 1;
}

/**
 * Reshape Tensor from Lua table.
 */
int tensorReshape(lua_State* L) {
    Tensor* tensor = Lua::get<Tensor*>(L, 1);
    if (!tensor) {
        return luaL_error(L, "reshape: expected Tensor");
    }
    
    std::vector<int> new_shape;
    if (lua_istable(L, 2)) {
        size_t len = lua_rawlen(L, 2);
        for (size_t i = 1; i <= len; i++) {
            lua_rawgeti(L, 2, i);
            new_shape.push_back(static_cast<int>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }
    }
    
    tensor->reshapeCpp(new_shape);
    return 0;
}

/**
 * Factory function to create Tensor from Lua table of shape.
 * Note: First arg is the class metatable, second arg (if present) is the shape.
 */
int createTensor(lua_State* L) {
    std::vector<int> shape;
    
    int nargs = lua_gettop(L);
    
    // Argument 2 (if present) is the shape table
    if (nargs >= 2 && lua_istable(L, 2)) {
        size_t len = lua_rawlen(L, 2);
        for (size_t i = 1; i <= len; i++) {
            lua_rawgeti(L, 2, i);
            int val = static_cast<int>(lua_tointeger(L, -1));
            shape.push_back(val);
            lua_pop(L, 1);
        }
    }
    
    Tensor tensor(shape);
    Lua::push(L, tensor);
    return 1;
}

} // namespace PostLib

/**
 * Phase 2.3: Shared pointer lifecycle test
 */
class Tracked {
public:
    static int alive_count;
    Tracked() { alive_count++; }
    ~Tracked() { alive_count--; }
    int getValue() const { return 42; }
};

int Tracked::alive_count = 0;

int makeTracked(lua_State* L) { 
    auto tracked = std::make_shared<Tracked>();
    Lua::push(L, tracked);
    return 1;
}

int aliveCount(lua_State* L) { 
    lua_pushinteger(L, Tracked::alive_count);
    return 1;
}

/**
 * Register PostLib module to Lua.
 */
extern "C" int luaopen_PostLib(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    
    LuaBinding(mod)
        .beginClass<Tensor>("Tensor")
            .addFactory(&createTensor)
            .addFunction("getShape", [](Tensor* t, lua_State* L) -> int {
                std::vector<int> shape = t->getShapeCpp();
                LuaRef result = LuaRef::createTable(L);
                for (size_t i = 0; i < shape.size(); i++) {
                    result.set(i + 1, shape[i]);
                }
                result.pushToStack();
                return 1;
            })
            .addFunction("ndim", &Tensor::ndim)
            .addFunction("empty", &Tensor::empty)
            .addFunction("clone", &Tensor::clone)
            .addFunction("fill", &Tensor::fill)
            .addFunction("reshape", [](Tensor* t, lua_State* L) -> int {
                std::vector<int> new_shape;
                if (lua_istable(L, 2)) {
                    size_t len = lua_rawlen(L, 2);
                    for (size_t i = 1; i <= len; i++) {
                        lua_rawgeti(L, 2, i);
                        new_shape.push_back(static_cast<int>(lua_tointeger(L, -1)));
                        lua_pop(L, 1);
                    }
                }
                t->reshapeCpp(new_shape);
                return 0;
            })
            .addFunction("at", static_cast<double(Tensor::*)(size_t)const>(&Tensor::at))
            // Add __len metamethod for #tensor
            .addFunction("__len", &Tensor::length)
        .endClass()
        
        .beginClass<Box>("Box")
            .addConstructor(LUA_ARGS(_opt<float>, _opt<float>, _opt<float>, _opt<float>, _opt<float>, _opt<int>))
            .addProperty("x1", &Box::getX1)
            .addProperty("y1", &Box::getY1)
            .addProperty("x2", &Box::getX2)
            .addProperty("y2", &Box::getY2)
            .addProperty("confidence", &Box::getConfidence)
            .addProperty("classId", &Box::getClassId)
        .endClass()
        
        .addFunction("parseYoloOutput", &parseYoloOutput)
        .addFunction("nms", &nms)
        .addFunction("scaleBoxes", &scaleBoxes)
        
        // Phase 2.3 test functions
        .addFunction("createTestBoxes", &createTestBoxes)
        .addFunction("consumeBoxTable", &consumeBoxTable)
        .addFunction("createEmptyBoxes", &createEmptyBoxes)
        .addFunction("createLargeBoxes", &createLargeBoxes)
        
        // Shared_ptr lifecycle tests
        .beginClass<Tracked>("Tracked")
            .addConstructor(LUA_SP(std::shared_ptr<Tracked>), LUA_ARGS())
            .addFunction("getValue", &Tracked::getValue)
        .endClass()
        .addFunction("makeTracked", &makeTracked)
        .addFunction("aliveCount", &aliveCount);
    
    mod.pushToStack();
    return 1;
}
