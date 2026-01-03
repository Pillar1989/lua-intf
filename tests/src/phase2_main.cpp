#include "common_bindings.h"
#include "cv_types.h"
#include "post_types.h"
#include "LuaIntf.h"
#include <iostream>
#include <cstdlib>

using namespace LuaIntf;

// Module registration functions
extern "C" int luaopen_CVLib(lua_State* L);
extern "C" int luaopen_PostLib(lua_State* L);
extern "C" int luaopen_Test(lua_State* L);

int main(int argc, char* argv[]) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    // Register CVLib module
    luaopen_CVLib(L);
    lua_setglobal(L, "CVLib");
    
    // Register PostLib module
    luaopen_PostLib(L);
    lua_setglobal(L, "PostLib");
    
    // Register Test module (for advanced feature tests)
    luaopen_Test(L);
    lua_setglobal(L, "Test");
    
    // Execute script if provided
    if (argc > 1) {
        if (luaL_dofile(L, argv[1]) != LUA_OK) {
            std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_close(L);
            return 1;
        }
    } else {
        // Interactive REPL
        std::cout << "Lua " << LUA_VERSION << " with CVLib and PostLib\n";
        std::cout << "Enter Lua code (Ctrl+D to exit):\n";
        
        while (true) {
            std::cout << "> ";
            std::string line;
            if (!std::getline(std::cin, line)) break;
            
            if (luaL_dostring(L, line.c_str()) != LUA_OK) {
                std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        }
    }
    
    lua_close(L);
    return 0;
}
