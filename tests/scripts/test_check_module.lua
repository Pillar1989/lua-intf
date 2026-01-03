-- Quick test to check Test module registration
print("Testing Test module registration...")
print("Test =", Test)
if Test then
    print("Test module exists")
    for k, v in pairs(Test) do
        print("  " .. k, v)
    end
else
    print("ERROR: Test module not found")
end
