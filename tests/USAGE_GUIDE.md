# lua-intf Complete Usage Guide

This guide demonstrates how to use lua-intf to bind C++ code to Lua, based on real working examples from this test suite.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Basic Binding Patterns](#basic-binding-patterns)
3. [Module Organization](#module-organization)
4. [Class Binding](#class-binding)
5. [Function Binding](#function-binding)
6. [Property and Field Binding](#property-and-field-binding)
7. [Container Conversion](#container-conversion)
8. [Memory Management](#memory-management)
9. [Advanced Features](#advanced-features)
10. [Best Practices](#best-practices)
11. [Debugging Tips](#debugging-tips)

## Getting Started

### Prerequisites

**CRITICAL**: Compile Lua as C++ for exception safety:

```bash
cd lua
make clean
make CC="c++" MYCFLAGS="-x c++ -DLUA_USE_POSIX" MYLIBS="" all
```

This ensures proper exception handling between Lua and C++.

### Basic Setup

```cpp
#include "LuaIntf.h"

using namespace LuaIntf;

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    // Your bindings here
    
    lua_close(L);
    return 0;
}
```

Compile with:
```bash
c++ -std=c++20 -I lua-intf/src/include -DLUAINTF_HEADERS_ONLY=1 \
    main.cpp lua/liblua.a -o myapp
```

## Basic Binding Patterns

### 1. Module Export

**Pattern**: Export a module that Lua can require or use directly.

```cpp
extern "C" int luaopen_MyModule(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    
    LuaBinding(mod)
        .beginModule("MyModule")
            .addFunction("hello", []() { return "Hello, Lua!"; })
        .endModule();
    
    mod.pushToStack();
    return 1;
}

// In main():
luaopen_MyModule(L);
lua_setglobal(L, "MyModule");
```

**Lua usage**:
```lua
local result = MyModule.hello()  -- "Hello, Lua!"
```

**Real example**: [src/cv_module.cpp](src/cv_module.cpp) - CVLib module

### 2. Class Binding

**Pattern**: Bind a C++ class with constructors, methods, and properties.

```cpp
class Image {
private:
    int width_, height_;
public:
    Image(int w, int h) : width_(w), height_(h) {}
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    void resize(int w, int h) { width_ = w; height_ = h; }
};

LuaBinding(mod)
    .beginClass<Image>("Image")
        .addConstructor(LUA_ARGS(int, int))
        .addProperty("width", &Image::getWidth)
        .addProperty("height", &Image::getHeight)
        .addFunction("resize", &Image::resize)
    .endClass();
```

**Lua usage**:
```lua
local img = Image(640, 480)
print(img.width)       -- 640
img:resize(1024, 768)
print(img.height)      -- 768
```

**Real example**: [include/cv_types.h](include/cv_types.h) - Image class

## Module Organization

### Flat Module (Direct Functions)

Best for utility functions or when you don't need namespacing:

```cpp
extern "C" int luaopen_Utils(lua_State* L) {
    LuaRef mod = LuaRef::createTable(L);
    
    LuaBinding(mod)
        .addFunction("add", [](int a, int b) { return a + b; })
        .addFunction("multiply", [](int a, int b) { return a * b; });
    
    mod.pushToStack();
    return 1;
}
```

**Lua**: `Utils.add(2, 3)` ✓ (not `Utils.Utils.add()`)

### Nested Module

When you need sub-namespaces:

```cpp
LuaBinding(mod)
    .beginModule("Math")
        .addFunction("add", &add)
        .beginModule("Advanced")
            .addFunction("sqrt", &sqrt)
        .endModule()
    .endModule();
```

**Lua**: `Math.Advanced.sqrt(16)`

**Real example**: [src/test_module.cpp](src/test_module.cpp) - Test module (flat organization)

## Class Binding

### Read-Write Properties

```cpp
class Person {
    std::string name_;
public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& n) { name_ = n; }
};

LuaBinding(mod)
    .beginClass<Person>("Person")
        .addProperty("name", &Person::getName, &Person::setName)
    .endClass();
```

**Lua**:
```lua
person.name = "Alice"  -- calls setName()
print(person.name)     -- calls getName()
```

### Read-Only Properties

```cpp
LuaBinding(mod)
    .beginClass<Person>("Person")
        .addProperty("name", &Person::getName)  -- No setter
    .endClass();
```

### Member Fields (Direct Access)

```cpp
struct Point {
    float x, y;
};

LuaBinding(mod)
    .beginClass<Point>("Point")
        .addVariable("x", &Point::x)  // By-value
        .addVariable("y", &Point::y)
    .endClass();
```

**Lua**: `point.x = 10`

### Metamethods

```cpp
LuaBinding(mod)
    .beginClass<Array>("Array")
        .addMetaFunction("__len", +[](Array* arr) { 
            return static_cast<int>(arr->size()); 
        })
        .addMetaFunction("__tostring", +[](Array* arr) {
            return std::string("Array[") + std::to_string(arr->size()) + "]";
        })
    .endClass();
```

**Note**: Use `+[](...)` lambda syntax for metamethods to avoid template errors.

**Real example**: [src/test_module.cpp](src/test_module.cpp) - TensorView `__len`

## Function Binding

### Normal Functions

```cpp
std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

LuaBinding(mod)
    .addFunction("greet", &greet);
```

### Lambda Functions

```cpp
LuaBinding(mod)
    .addFunction("square", [](int x) { return x * x; });
```

### Function Overloads

Use explicit casts to disambiguate:

```cpp
int add(int a, int b) { return a + b; }
float add(float a, float b) { return a + b; }

LuaBinding(mod)
    .addFunction("addInt", static_cast<int(*)(int,int)>(&add))
    .addFunction("addFloat", LUA_FN(float, add, float, float));
```

### Optional Arguments

```cpp
std::string greet(const std::string& name, const std::string& title = "Mr.") {
    return title + " " + name;
}

LuaBinding(mod)
    .addFunction("greet", &greet, LUA_ARGS(std::string, _opt<std::string>));
```

**Lua**:
```lua
greet("Smith")           -- "Mr. Smith"
greet("Smith", "Dr.")    -- "Dr. Smith"
```

### Default Values

```cpp
LuaBinding(mod)
    .addFunction("power", &power, LUA_ARGS(float, _def<int, 2>));
    // power(x, n=2)
```

**Argument Modifiers**:
- `_opt<TYPE>` - Optional, uses default constructor
- `_def<TYPE, NUM, DEN=1>` - Optional with default value
- `_out<TYPE&>` - Output-only reference
- `_ref<TYPE&>` - Input/output reference
- `_ref_opt<TYPE&>` - Optional input/output reference

**Real example**: [src/cv_module.cpp](src/cv_module.cpp) - Image constructor with optional channels

### lua_CFunction Convention

For manual stack manipulation:

```cpp
int myFunction(lua_State* L) {
    int arg1 = Lua::get<int>(L, 1);
    std::string arg2 = Lua::get<std::string>(L, 2);
    
    // Do work...
    
    Lua::push(L, result);
    return 1;  // Number of return values
}

LuaBinding(mod)
    .addFunction("myFunction", &myFunction);
```

**Real example**: [src/cv_module.cpp](src/cv_module.cpp) - normalize() and hwc2chw()

## Property and Field Binding

### Static Properties

```cpp
class Config {
public:
    static std::string getVersion() { return "1.0"; }
    static void setDebug(bool d) { debug_ = d; }
private:
    static bool debug_;
};

LuaBinding(mod)
    .beginClass<Config>("Config")
        .addStaticProperty("version", &Config::getVersion)
        .addStaticProperty("debug", nullptr, &Config::setDebug)  // Write-only
    .endClass();
```

**Lua**: `Config.version` or `Config.debug = true`

### Static Variables

```cpp
static int counter = 0;

LuaBinding(mod)
    .beginModule("Utils")
        .addVariable("counter", &counter)
    .endModule();
```

## Container Conversion

### std::vector ↔ Lua table

Enable conversion:

```cpp
namespace LuaIntf {
    LUA_USING_LIST_TYPE(std::vector)
}

std::vector<int> getNumbers() {
    return {1, 2, 3, 4, 5};
}

void processNumbers(const std::vector<int>& nums) {
    // Automatically converts from Lua table
}

LuaBinding(mod)
    .addFunction("getNumbers", &getNumbers)
    .addFunction("processNumbers", &processNumbers);
```

**Lua**:
```lua
local nums = getNumbers()  -- Lua table {1, 2, 3, 4, 5}
processNumbers({10, 20, 30})  -- Pass Lua table to C++
```

**Real example**: [include/common_bindings.h](include/common_bindings.h)

### Nested Containers

Automatically supported when using `<vector>`:

```cpp
std::vector<std::vector<int>> createMatrix() {
    return {{1, 2}, {3, 4}, {5, 6}};
}

LuaBinding(mod)
    .addFunction("createMatrix", &createMatrix);
```

**Lua**:
```lua
local matrix = createMatrix()  -- {{1,2}, {3,4}, {5,6}}
print(matrix[1][2])  -- 2
```

**Real example**: [src/test_module.cpp](src/test_module.cpp) - createNested()

### Custom Container Types

```cpp
namespace LuaIntf {
    LUA_USING_LIST_TYPE(std::deque)
    LUA_USING_MAP_TYPE(std::map)
}
```

## Memory Management

### By Value (Copy)

```cpp
Image createImage() {
    return Image(640, 480);  // Copied to Lua
}
```

Lua owns the copy, no C++ dependency.

### By Pointer (Reference)

```cpp
Image* globalImage = new Image(640, 480);

Image* getImage() {
    return globalImage;  // Lua stores pointer
}
```

⚠️ **Danger**: Object must outlive Lua access. Lua won't delete it.

### Shared Pointer (Shared Ownership)

**Enable shared_ptr**:

```cpp
namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}

LuaBinding(mod)
    .beginClass<Image>("Image")
        .addConstructor(LUA_SP(std::shared_ptr<Image>), LUA_ARGS(int, int))
    .endClass();

std::shared_ptr<Image> createImage() {
    return std::make_shared<Image>(640, 480);
}
```

Both C++ and Lua can safely hold the object. Deleted when last reference goes away.

**Real example**: [src/test_module.cpp](src/test_module.cpp) - TensorView with shared_ptr

### Custom Deleter

```cpp
struct MyDeleter {
    void operator()(MyClass* p) {
        p->cleanup();
        delete p;
    }
};

LuaBinding(mod)
    .beginClass<MyClass>("MyClass")
        .addConstructor(LUA_DEL(MyDeleter), LUA_ARGS(int))
    .endClass();
```

## Advanced Features

### Zero-Copy TensorView

For large arrays without copying:

```cpp
template<typename T>
class TensorView {
    T* data_;
    size_t length_;
    std::shared_ptr<void> owner_;  // Keep data alive
public:
    TensorView(T* data, size_t len, std::shared_ptr<void> owner)
        : data_(data), length_(len), owner_(owner) {}
    
    T get(int idx) const { return data_[idx - 1]; }  // Lua 1-based
    void set(int idx, T val) { data_[idx - 1] = val; }
    int length() const { return static_cast<int>(length_); }
};

LuaBinding(mod)
    .beginClass<TensorView<float>>("FloatTensorView")
        .addFunction("get", &TensorView<float>::get)
        .addFunction("set", &TensorView<float>::set)
        .addMetaFunction("__len", +[](TensorView<float>* v) { 
            return v->length(); 
        })
    .endClass();
```

**Lua**:
```lua
local view = createView()  -- Wraps C++ array
print(#view)               -- 10000000 (no copy!)
view:set(1, 3.14)
print(view:get(1))         -- 3.14
```

**Real example**: [src/include/impl/TensorView.h](../src/include/impl/TensorView.h)

### Table Mutation

LuaRef preserves table identity:

```cpp
void removeEvens(LuaRef table) {
    for (int i = table.length(); i >= 1; i--) {
        if (table[i].toValue<int>() % 2 == 0) {
            table.rawset(i, LuaRef());  // Set to nil
        }
    }
    table.compact();  // Remove nils
}
```

**Lua**:
```lua
local t = {1, 2, 3, 4, 5}
removeEvens(t)
-- t is now {1, 3, 5}, same table object
```

**Real example**: [src/post_module.cpp](src/post_module.cpp) - nms()

### Field Name Compatibility

Handle multiple naming conventions:

```cpp
void processBox(LuaRef box) {
    // Support both class_id and classId
    int class_id = box.has("class_id") ? 
        box.get<int>("class_id") : 
        box.get<int>("classId");
}
```

**Real example**: [src/post_module.cpp](src/post_module.cpp) - nms()

## Best Practices

### 1. Always Check Field Existence

❌ **Bad**:
```cpp
int value = luaRef.get<int>("field");  // Crashes if missing
```

✅ **Good**:
```cpp
if (!luaRef.has("field")) {
    throw std::runtime_error("Missing field");
}
int value = luaRef.get<int>("field");
```

### 2. Handle Empty/Invalid Input

❌ **Bad**:
```cpp
void process(const Image& img) {
    uint8_t* data = img.data();  // Crashes if empty
    // ...
}
```

✅ **Good**:
```cpp
void process(const Image& img) {
    if (img.empty() || img.getWidth() <= 0) {
        return;  // Or return empty result
    }
    // ...
}
```

**Real example**: [src/cv_module.cpp](src/cv_module.cpp) - normalize() and hwc2chw()

### 3. Use Appropriate Memory Management

- **By value**: Small objects, clear ownership
- **By pointer**: Objects with external lifetime
- **Shared pointer**: Shared ownership between C++ and Lua

### 4. Compile Lua as C++

Always use `-x c++` when building Lua for exception safety.

### 5. Be Consistent with Naming

Choose one convention (snake_case or camelCase) and stick to it, or support both.

## Debugging Tips

### 1. "bad extra argument" Errors

Usually means **missing table field**, not a lua-intf bug.

**Solution**: Use `has()` before `get()`.

### 2. Segmentation Faults

Common causes:
- Accessing empty images/containers
- Null pointer dereference
- Accessing deleted objects

**Solution**: Enable AddressSanitizer:
```bash
CXXFLAGS="-g -fsanitize=address" make
```

### 3. Type Conversion Errors

lua-intf is strict about types.

**Solution**: Check types with `luaRef.type()` or add explicit conversions.

### 4. Template Instantiation Errors

For metamethods, use lambda with `+` prefix:

```cpp
.addMetaFunction("__len", +[](MyClass* obj) { return obj->size(); })
```

### 5. Checking lua-intf Modifications

See [../MODIFICATIONS.md](../MODIFICATIONS.md) for all changes and verified features.

## Complete Examples

### Example 1: Simple Module

See [src/test_main.cpp](src/test_main.cpp) for a minimal setup.

### Example 2: Computer Vision Module

See [src/cv_module.cpp](src/cv_module.cpp) for a complete module with:
- Multiple classes (Image)
- Image processing functions
- Boundary checking
- lua_CFunction convention

### Example 3: Post-Processing Module

See [src/post_module.cpp](src/post_module.cpp) for:
- In-place table mutation (NMS)
- Field compatibility
- Box coordinate operations

### Example 4: Advanced Features

See [src/test_module.cpp](src/test_module.cpp) for:
- Zero-copy TensorView
- Nested container conversion
- Shared pointer usage

## Running the Examples

```bash
cd lua-intf/tests

# Build
make

# Run basic tests
make test_basic

# Run advanced tests
make test_advanced

# Run specific test
./phase2_cli scripts/test_tensor_view.lua
```

## Further Reading

- **[../README.md](../README.md)** - lua-intf main documentation
- **[../MODIFICATIONS.md](../MODIFICATIONS.md)** - All modifications and investigation results
- **[scripts/](scripts/)** - All test scripts with inline comments

## Getting Help

If you encounter issues:

1. Check [../MODIFICATIONS.md](../MODIFICATIONS.md) for known issues
2. Run tests to see working examples
3. Enable debug symbols and AddressSanitizer
4. Verify Lua is compiled as C++

---

**Last Updated**: 2026-01-03

This guide is based on real working code. All examples can be found in [src/](src/) and tested via [scripts/](scripts/).
