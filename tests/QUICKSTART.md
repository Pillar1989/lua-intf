# lua-intf Test Suite - Quick Reference

## Directory Layout

```
lua-intf/
├── tests/                    # ← You are here
│   ├── Makefile             # Build system
│   ├── README.md            # Test suite overview
│   ├── USAGE_GUIDE.md       # Complete usage guide (START HERE!)
│   ├── scripts/             # 16 Lua test scripts
│   ├── src/                 # C++ module implementations
│   └── include/             # Header files
├── src/include/             # lua-intf headers
│   └── impl/
│       └── TensorView.h     # Zero-copy array wrapper
├── MODIFICATIONS.md         # All changes and investigations
└── README.md               # Main documentation
```

## Running Tests

```bash
cd lua-intf/tests

# Build all
make

# Run all tests
make test

# Run by category
make test_basic        # __len, vector, table mutation, shared_ptr
make test_advanced     # TensorView, nested containers, edge cases
make test_integration  # CVLib + PostLib modules

# Run individual tests
./test_cli scripts/test_vector_conversion.lua
./phase2_cli scripts/test_edge_cases.lua
```

## Test Categories

### Basic Features (test_cli)
- `test_len_metamethod.lua` - Custom __len operator
- `test_vector_conversion.lua` - std::vector ↔ Lua table
- `test_table_mutation.lua` - In-place Lua table modification
- `test_shared_ownership.lua` - shared_ptr lifecycle

### Advanced Features (phase2_cli)
- `test_tensor_view.lua` - Zero-copy 10M element array
- `test_nested_containers.lua` - vector<vector<T>> conversion
- `test_edge_cases.lua` - 24 boundary tests

### Integration Tests (phase2_cli)
- `test_cvlib.lua` - Computer vision operations
- `test_postlib.lua` - Post-processing (NMS, box scaling)

## Learning Path

1. **Read [USAGE_GUIDE.md](USAGE_GUIDE.md)** (30 min)
   - Complete guide with all binding patterns
   - Real examples from working code
   
2. **Explore test scripts** (20 min)
   - See Lua-side usage in `scripts/`
   - Understand expected behavior

3. **Study module implementations** (30 min)
   - `src/test_main.cpp` - Minimal setup
   - `src/cv_module.cpp` - Full module example
   - `src/test_module.cpp` - Advanced features

4. **Run tests** (5 min)
   ```bash
   make test
   ```

5. **Experiment** (∞)
   - Modify tests
   - Add your own modules
   - See what works!

## Key Concepts

### Module Organization
```cpp
extern "C" int luaopen_MyModule(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    LuaBinding(mod)
        .addFunction("hello", []() { return "world"; });
    mod.pushToStack();
    return 1;
}
```

### Class Binding
```cpp
LuaBinding(mod)
    .beginClass<Image>("Image")
        .addConstructor(LUA_ARGS(int, int))
        .addProperty("width", &Image::getWidth)
        .addFunction("resize", &Image::resize)
    .endClass();
```

### Container Conversion
```cpp
namespace LuaIntf {
    LUA_USING_LIST_TYPE(std::vector)
}
// Now std::vector<T> ↔ Lua table automatically!
```

### Memory Management
- **By value**: `Image createImage()` - Lua owns copy
- **By pointer**: `Image* getImage()` - C++ owns, Lua references
- **Shared pointer**: `shared_ptr<Image>` - Shared ownership

## Common Patterns

See [USAGE_GUIDE.md](USAGE_GUIDE.md) sections:
- [Basic Binding Patterns](USAGE_GUIDE.md#basic-binding-patterns)
- [Class Binding](USAGE_GUIDE.md#class-binding)
- [Function Binding](USAGE_GUIDE.md#function-binding)
- [Container Conversion](USAGE_GUIDE.md#container-conversion)
- [Memory Management](USAGE_GUIDE.md#memory-management)

## Debugging

### Build Errors
```bash
# Ensure Lua is built as C++
cd ../../lua
make clean
make CC="c++" MYCFLAGS="-x c++ -DLUA_USE_POSIX" all
```

### Runtime Crashes
```bash
# Enable sanitizers
CXXFLAGS="-g -fsanitize=address" make
```

### "bad extra argument" Errors
→ Usually missing table field, NOT lua-intf bug
→ Use `luaRef.has("field")` before `get()`

## Test Results

All tests passing (2026-01-03):
- ✅ Basic features: 4/4
- ✅ Advanced features: 24 edge cases, TensorView, nested containers
- ✅ Integration: CVLib + PostLib

## Documentation

| Document | Purpose |
|----------|---------|
| [USAGE_GUIDE.md](USAGE_GUIDE.md) | Complete usage guide (read first!) |
| [README.md](README.md) | Test suite overview |
| [../MODIFICATIONS.md](../MODIFICATIONS.md) | All changes and investigations |
| [../README.md](../README.md) | lua-intf main docs |

## Files

```
scripts/  (16 Lua test scripts)
├── test_len_metamethod.lua
├── test_vector_conversion.lua
├── test_table_mutation.lua
├── test_shared_ownership.lua
├── test_tensor_view.lua
├── test_nested_containers.lua
├── test_edge_cases.lua
├── test_cvlib.lua
├── test_postlib.lua
└── ... (7 more)

src/  (5 C++ implementations)
├── test_main.cpp          # Minimal CLI
├── phase2_main.cpp        # Full-featured CLI
├── cv_module.cpp          # Computer vision module
├── post_module.cpp        # Post-processing module
└── test_module.cpp        # Advanced features

include/  (3 headers)
├── cv_types.h
├── post_types.h
└── common_bindings.h
```

## Next Steps

1. Read [USAGE_GUIDE.md](USAGE_GUIDE.md)
2. Run `make test`
3. Study examples in `src/`
4. Build your own module!

---

**Questions?** See [USAGE_GUIDE.md](USAGE_GUIDE.md) or [../MODIFICATIONS.md](../MODIFICATIONS.md)

**Found an issue?** Check [../MODIFICATIONS.md](../MODIFICATIONS.md) for known issues and solutions
