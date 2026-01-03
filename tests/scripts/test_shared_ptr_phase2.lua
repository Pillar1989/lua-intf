-- Test shared_ptr lifetime management (Phase 2.3)

print("=== Testing shared_ptr lifecycle ===")

-- Initial count should be 0
local initial = PostLib.aliveCount()
print("Initial alive count: " .. initial)
assert(initial == 0, "Initial count should be 0, got " .. initial)

-- Create object via constructor (Lua holds shared_ptr copy, ref_count = 1)
print("\nCreating Tracked object via constructor...")
local obj = PostLib.Tracked()
assert(obj ~= nil, "Tracked() should return object")

local after_create = PostLib.aliveCount()
print("After creation: " .. after_create)
assert(after_create == 1, "Object not created, count = " .. after_create)

-- Test that object is functional
print("\nTesting object functionality...")
local value = obj:getValue()
print("Object getValue() = " .. value)
assert(value == 42, "getValue should return 42")

-- C++ side already released its shared_ptr after return
-- Object stays alive because Lua's userdata holds a shared_ptr
print("\nObject is alive in Lua (managed by shared_ptr)")

-- Clear Lua reference
print("\nClearing Lua reference...")
obj = nil

-- Force garbage collection
print("Running garbage collection...")
collectgarbage("collect")
collectgarbage("collect")  -- Call twice to ensure finalization

local after_gc = PostLib.aliveCount()
print("After GC: " .. after_gc)
assert(after_gc == 0, "Object not destroyed after GC, count = " .. after_gc)

print("\n✓ Shared pointer test PASSED")
print("  - Object created with shared_ptr via LUA_SP constructor")
print("  - Lua kept object alive via shared_ptr in userdata")
print("  - GC properly destroyed object when Lua reference cleared")

