-- Test shared_ptr lifetime management

print("=== Testing shared_ptr lifecycle ===")

-- Initial count should be 0
assert(Test.aliveCount() == 0, "Initial count wrong")
print("Initial alive count:", Test.aliveCount())

-- Create object (Lua holds shared_ptr copy, ref_count = 1)
local obj = Test.makeTracked()
assert(Test.aliveCount() == 1, "Object not created")
print("After creation:", Test.aliveCount())

-- C++ side already released its shared_ptr after return
-- Object stays alive because Lua's userdata holds a shared_ptr

-- Clear Lua reference
obj = nil

-- Force garbage collection
collectgarbage("collect")
collectgarbage("collect")  -- Call twice to ensure finalization

assert(Test.aliveCount() == 0, "Object not destroyed after GC")
print("After GC:", Test.aliveCount())

print("✓ Shared pointer test PASSED")
