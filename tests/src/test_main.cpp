#include "common_bindings.h"
#include <cassert>
#include <iostream>

using namespace LuaIntf;

// Test class for __len metamethod
class TestLen {
public:
    int size() const { return 42; }
};

// Test class for shared_ptr lifecycle tracking
class Tracked {
public:
    static int alive_count;
    Tracked() { alive_count++; }
    ~Tracked() { alive_count--; }
};
int Tracked::alive_count = 0;

// Test functions for vector conversion
std::vector<int> getVec() {
    return {1, 2, 3};
}

void consumeVec(const std::vector<int>& v) {
    assert(v.size() == 3);
    assert(v[0] == 10 && v[1] == 20 && v[2] == 30);
}

// Test functions for nested containers
std::vector<std::vector<int>> createNested() {
    return {{1, 2, 3}, {4, 5}, {6, 7, 8, 9}};
}

void consumeNested(const std::vector<std::vector<int>>& vv) {
    assert(vv.size() == 2);
    assert(vv[0].size() == 3 && vv[0][0] == 10);
    assert(vv[1].size() == 2 && vv[1][1] == 21);
}

// Test function for table mutation
void modifyTable(LuaRef table) {
    // Remove element at index 2 and compact
    table.removeAt(2);  // nil out index 2
    table.compact();    // shift elements and reduce size
}

// Test function for shared_ptr
std::shared_ptr<Tracked> makeTracked() {
    return std::make_shared<Tracked>();
}

int aliveCount() {
    return Tracked::alive_count;
}

// Test function for TensorView
std::shared_ptr<std::vector<float>> g_data;  // Keep data alive

TensorView<float> createView() {
    if (!g_data) {
        g_data = std::make_shared<std::vector<float>>(10000000, 3.14f);
    }
    return TensorView<float>(g_data->data(), g_data->size(), g_data);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_cli <script.lua>" << std::endl;
        return 1;
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // Register TestLen class with __len metamethod
    LuaBinding(L).beginClass<TestLen>("TestLen")
        .addConstructor(LUA_ARGS())
        .addFunction("__len", &TestLen::size)
    .endClass();

    // Register Tracked class with shared_ptr support
    LuaBinding(L).beginClass<Tracked>("Tracked")
        .addConstructor(LUA_SP(std::shared_ptr<Tracked>), LUA_ARGS())
    .endClass();

    // Register TensorView class
    LuaBinding(L).beginClass<TensorView<float>>("FloatTensorView")
        .addConstructor(LUA_ARGS())
        .addFunction("get", &TensorView<float>::get)
        .addFunction("set", &TensorView<float>::set)
        .addFunction("__len", &TensorView<float>::length)
    .endClass();

    // Register Test module with all test functions
    LuaBinding(L).beginModule("Test")
        .addFunction("getVec", &getVec)
        .addFunction("consumeVec", &consumeVec)
        .addFunction("createNested", &createNested)
        .addFunction("consumeNested", &consumeNested)
        .addFunction("modifyTable", &modifyTable)
        .addFunction("makeTracked", &makeTracked)
        .addFunction("aliveCount", &aliveCount)
        .addFunction("createView", &createView)
    .endModule();

    // Run the specified Lua script
    if (luaL_dofile(L, argv[1]) != LUA_OK) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return 1;
    }

    lua_close(L);
    return 0;
}
