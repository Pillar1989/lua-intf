// Test module for advanced lua-intf features

#include "LuaIntf.h"
#include <vector>
#include <memory>

using namespace LuaIntf;

// Enable vector conversion (required for nested containers)
namespace LuaIntf {
    LUA_USING_LIST_TYPE(std::vector)
}

// Create nested vector for testing nested container conversion
static std::vector<std::vector<int>> createNested() {
    return {
        {1, 2, 3},
        {4, 5},
        {6, 7, 8, 9}
    };
}

// Create TensorView for testing zero-copy access
#include "impl/TensorView.h"

static TensorView<float> createView() {
    // Create 10M element array
    auto data = std::make_shared<std::vector<float>>(10000000, 3.14f);
    return TensorView<float>(data->data(), data->size(), data);
}

// Test consuming nested vector from Lua
static int consumeNested(const std::vector<std::vector<int>>& nested) {
    int total = 0;
    for (const auto& inner : nested) {
        for (int val : inner) {
            total += val;
        }
    }
    return total;
}

extern "C" int luaopen_Test(lua_State* L)
{
    LuaRef mod = LuaRef::createTable(L);
    
    // Bind functions directly to module (not nested)
    LuaBinding(mod)
        .addFunction("createNested", &createNested)
        .addFunction("createView", &createView)
        .addFunction("consumeNested", &consumeNested);
    
    // Bind TensorView class
    LuaBinding(mod)
        .beginClass<TensorView<float>>("FloatTensorView")
            .addConstructor(LUA_ARGS())
            .addFunction("get", &TensorView<float>::get)
            .addFunction("set", &TensorView<float>::set)
            .addMetaFunction("__len", +[](TensorView<float>* view) -> int {
                return view->length();
            })
        .endClass();
    
    mod.pushToStack();
    return 1;
}
