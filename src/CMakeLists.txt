cmake_minimum_required(VERSION 3.18)
project(LuaIntf LANGUAGES C CXX)

include(GNUInstallDirs)
include(FetchContent)

option(ENABLE_BENCH "Build Benchmarks")
option(BUNDLE_LUA "Use a bundled Lua" ON)

set(luaIntf_TARGETS LuaIntf)

if(BUNDLE_LUA)
    message(STATUS "Building Lua from source")

    FetchContent_Declare(
        lua
        URL https://www.lua.org/ftp/lua-5.5.0.tar.gz
        URL_HASH SHA256=57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    FetchContent_MakeAvailable(lua)
    file(GLOB LUA_SRC "${lua_SOURCE_DIR}/src/*.c")
    list(REMOVE_ITEM LUA_SRC "${lua_SOURCE_DIR}/src/lua.c" "${lua_SOURCE_DIR}/src/luac.c")
    add_library(lua STATIC ${LUA_SRC})
    target_compile_options(lua PRIVATE -x c++ -fPIC -g -O3 -fno-omit-frame-pointer -DNDEBUG)
    target_compile_definitions(lua PUBLIC -DLUA_USE_POSIX)

    target_include_directories(lua PUBLIC
        $<BUILD_INTERFACE:${lua_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(lua PRIVATE m)
    endif()

    set(LUA_LIBRARIES lua)
    set(LUA_INCLUDE_DIR "${lua_SOURCE_DIR}/src")
    list(APPEND luaIntf_TARGETS lua)
else()
    find_package(Lua REQUIRED)
endif()

add_library(LuaIntf SHARED
    CppBindClass.cpp
    CppBindModule.cpp
    CppFunction.cpp
    CppObject.cpp
    LuaCompat.cpp
    LuaRef.cpp
    LuaState.cpp
)

target_link_libraries(LuaIntf
    PUBLIC
        ${LUA_LIBRARIES}
)

target_include_directories(LuaIntf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(ENABLE_BENCH)
    find_package(benchmark REQUIRED)
    target_compile_options(LuaIntf PRIVATE -fno-omit-frame-pointer -DNDEBUG -Werror -Wall -Wextra -g -O3)
    add_executable(bench
        bench/class.cpp
        bench/main.cpp
        bench/string.cpp
        bench/table.cpp
        bench/unordered_map.cpp
        bench/vector.cpp
    )
    target_compile_options(bench PRIVATE -fno-omit-frame-pointer -DNDEBUG -Werror -Wall -Wextra -g -O3)
    target_link_libraries(bench benchmark::benchmark LuaIntf)
endif()

install(
    FILES
        include/impl/CppArg.h
        include/impl/CppBindClass.h
        include/impl/CppBindModule.h
        include/impl/CppFunction.h
        include/impl/CppInvoke.h
        include/impl/CppObject.h
        include/impl/LuaException.h
        include/impl/LuaType.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/LuaIntf/impl
)

install(
    FILES
        include/LuaCompat.h
        include/LuaContext.h
        include/LuaIntf.h
        include/LuaRef.h
        include/LuaState.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/LuaIntf
)

if(BUNDLE_LUA)
    install(
        FILES
            ${lua_SOURCE_DIR}/src/lua.h
            ${lua_SOURCE_DIR}/src/luaconf.h
            ${lua_SOURCE_DIR}/src/lualib.h
            ${lua_SOURCE_DIR}/src/lauxlib.h
            ${lua_SOURCE_DIR}/src/lua.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

target_compile_definitions(LuaIntf PUBLIC -DLUAINTF_HEADERS_ONLY=0 LUAINTF_LINK_LUA_COMPILED_IN_CXX=1)
target_compile_features(LuaIntf PUBLIC cxx_std_20)

install(
    TARGETS ${luaIntf_TARGETS}
    EXPORT LuaIntfTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/LuaIntf)

install(
    EXPORT LuaIntfTargets
    FILE LuaIntfConfig.cmake
    NAMESPACE LuaIntf::
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(TARGETS ${luaIntf_TARGETS} NAMESPACE LuaIntf:: FILE LuaIntfTargets.cmake)
