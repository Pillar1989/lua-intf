-- Minimal test to locate GC warning source

print("Creating object...")
local obj = TestLen()
print("Object created")

print("Getting length...")
local len = #obj
print("Length:", len)

print("Setting obj to nil...")
obj = nil

print("Forcing GC...")
collectgarbage("collect")
collectgarbage("collect")

print("Test complete")
