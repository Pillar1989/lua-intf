# lua-intf Tests and Examples

This directory contains comprehensive tests and examples demonstrating how to use lua-intf for binding C++ code to Lua.

## Directory Structure

```
tests/
├── Makefile              # Build system for tests
├── README.md             # This file
├── USAGE_GUIDE.md        # Comprehensive usage guide
├── scripts/              # Lua test scripts
│   ├── test_*.lua        # Various feature tests
│   └── ...
├── src/                  # C++ source files
│   ├── test_main.cpp     # Basic test CLI
│   ├── phase2_main.cpp   # Full-featured test CLI
│   ├── cv_module.cpp     # Computer vision module example
│   ├── post_module.cpp   # Post-processing module example
│   └── test_module.cpp   # Advanced features module
└── include/              # Header files
    ├── cv_types.h        # CV module types
    ├── post_types.h      # Post-processing types
    └── common_bindings.h # Common binding configurations
```

## Quick Start

### Prerequisites

- C++17 or C++20 compiler (gcc/clang)
- Lua 5.3+ compiled as C++ (for exception safety)
- make utility

### Build

```bash
# Build all test executables
make

# Or build specific targets
make test_cli      # Basic features only
make phase2_cli    # Full features with modules
```

### Run Tests

```bash
# Run all tests
make test

# Run specific test suites
make test_basic        # Basic lua-intf features
make test_advanced     # Advanced features (TensorView, nested containers)
make test_integration  # Module integration tests

# Run individual test scripts
./test_cli scripts/test_vector_conversion.lua
./phase2_cli scripts/test_edge_cases.lua
```

## Test Categories

### Basic Features (`make test_basic`)

1. **__len metamethod support** - Custom length operator for classes
2. **std::vector conversion** - Automatic C++/Lua container conversion
3. **Table mutation** - In-place Lua table modification
4. **Shared pointer lifecycle** - Memory management with shared_ptr

### Advanced Features (`make test_advanced`)

1. **Zero-copy TensorView** - 10M element array access without copying
2. **Nested containers** - Recursive std::vector<std::vector<T>> conversion
3. **Edge cases** - 24 boundary tests for robustness

### Integration Tests (`make test_integration`)

1. **CVLib module** - Computer vision operations (imread, bgr2rgb, letterbox, etc.)
2. **PostLib module** - Post-processing (NMS, box scaling, etc.)

## Learning Path

1. **Start with USAGE_GUIDE.md** - Comprehensive guide to lua-intf
2. **Explore basic tests** - Understand core binding patterns
3. **Study module examples** - See real-world module implementations
4. **Run tests** - Verify your understanding

## Example: Creating Your Own Module

See [src/test_module.cpp](src/test_module.cpp) for a complete example:

```cpp
extern "C" int luaopen_MyModule(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    
    LuaBinding(mod)
        .beginClass<MyClass>("MyClass")
            .addConstructor(LUA_ARGS(int, std::string))
            .addProperty("name", &MyClass::getName)
            .addFunction("process", &MyClass::process)
        .endClass();
    
    mod.pushToStack();
    return 1;
}
```

## Documentation

- **[USAGE_GUIDE.md](USAGE_GUIDE.md)** - Complete usage guide with examples
- **[../MODIFICATIONS.md](../MODIFICATIONS.md)** - All lua-intf modifications and investigations
- **[../README.md](../README.md)** - lua-intf main documentation

## Test Coverage

- ✅ Basic features: 4 tests
- ✅ Advanced features: 3 test suites
- ✅ Edge cases: 24 boundary tests
- ✅ Integration: Full module tests

All tests pass with zero failures.

## Common Issues

### Build Errors

If you see linking errors:
```bash
# Ensure Lua is built as C++
cd ../../lua
make clean
make CC="c++" MYCFLAGS="-x c++ -DLUA_USE_POSIX" all
```

### Runtime Errors

If tests crash:
- Check that Lua was compiled as C++ (exception safety)
- Verify `LUAINTF_LINK_LUA_COMPILED_IN_CXX=1` is set
- Enable debug symbols: `CXXFLAGS="-g -fsanitize=address"`

## Contributing

When adding new tests:
1. Add Lua script to `scripts/`
2. Update Makefile test targets if needed
3. Document the test purpose in this README
4. Ensure test passes before committing

## License

Same as lua-intf parent project (MIT License).
