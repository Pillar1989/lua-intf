# lua-intf Investigation: LuaRef + float Parameter Combination

##⚠️ **FALSE ALARM - NOT A BUG**

## Problem Description (Initially Reported)

When binding a C++ function with signature `(LuaRef, float)` or `(LuaRef, double)`, lua-intf appeared to produce a runtime error:
```
bad extra argument #-1 to '?' (number expected, got nil)
```

## Investigation Result

After extensive testing, this was **NOT a lua-intf bug**. The actual problem was:

### Root Cause
**Field name mismatch between Lua test code and C++ binding code:**
- Lua test used: `classId`  
- C++ code expected: `class_id`

When `box_ref.get<int>("class_id")` was called on a Lua table that only had `classId`, it failed with a confusing error message about argument indexing.

### Confirmed Working Cases:
- ✅ `void func(LuaRef, float)` - Works perfectly
- ✅ `void func(LuaRef, double)` - Works perfectly  
- ✅ `int func(LuaRef, float)` - Works perfectly
- ✅ `int func(LuaRef, int)` - Works perfectly
- ✅ `LuaRef func(LuaRef, int, int, int, int, LuaRef)` - Works perfectly

## Resolution

Fixed by adding support for both field name conventions:
```cpp
// Support both "class_id" and "classId" for compatibility
int class_id = box_ref.has("class_id") ? 
    box_ref.get<int>("class_id") : 
    box_ref.get<int>("classId");
```

## Lesson Learned

When debugging lua-intf binding issues:
1. ✅ Check field name consistency between Lua and C++ first
2. ✅ Verify table structure matches expected schema
3. ✅ Use `LuaRef::has(key)` to check field existence before `get()`
4. ⚠️ Error messages can be misleading - "bad extra argument #-1" was actually a missing field issue

## Status

✅ **RESOLVED** - No lua-intf bug. User error in test data structure.

All parameter combinations with LuaRef work correctly in lua-intf.
