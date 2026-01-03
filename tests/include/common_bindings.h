#pragma once

#include "LuaIntf.h"
#include <memory>
#include <vector>

// Enable shared_ptr support across all bindings
namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
    LUA_USING_LIST_TYPE(std::vector)
}

// Note: Nested container support is automatically enabled in LuaType.h when <vector> is included
// Note: TensorView is automatically included via LuaIntf.h
