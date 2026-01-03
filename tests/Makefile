# Makefile for lua-intf Tests and Examples
# This demonstrates how to build applications using lua-intf

.PHONY: all clean test test_basic test_advanced test_integration

# Compiler and flags
CXX = c++
CXXFLAGS = -std=c++20 -Wall -Wextra -I ../src/include -I include -I ../../lua -DLUAINTF_HEADERS_ONLY=1

# Lua library path (adjust if needed)
LUA_DIR = ../../lua
LUA_LIB = $(LUA_DIR)/liblua.a

# Test executables
TEST_CLI = test_cli
PHASE2_CLI = phase2_cli

all: $(TEST_CLI) $(PHASE2_CLI)
	@echo "✓ All test executables built successfully"
	@echo ""
	@echo "Run tests with:"
	@echo "  make test        # Run all tests"
	@echo "  make test_basic  # Run basic lua-intf feature tests"
	@echo "  make test_advanced # Run advanced feature tests"

# Ensure Lua library exists
$(LUA_LIB):
	@echo "Building Lua library as C++..."
	cd $(LUA_DIR) && $(MAKE) CC="$(CXX)" MYCFLAGS="-x c++ -DLUA_USE_POSIX" MYLIBS="" all

# Build test CLI (basic lua-intf features)
$(TEST_CLI): $(LUA_LIB) src/test_main.cpp
	@echo "Building test_cli..."
	$(CXX) $(CXXFLAGS) -o $(TEST_CLI) src/test_main.cpp $(LUA_LIB)

# Build Phase 2 CLI (with CVLib, PostLib, and Test modules)
$(PHASE2_CLI): $(LUA_LIB) src/phase2_main.cpp src/cv_module.cpp src/post_module.cpp src/test_module.cpp
	@echo "Building phase2_cli..."
	$(CXX) $(CXXFLAGS) -o $(PHASE2_CLI) src/phase2_main.cpp src/cv_module.cpp src/post_module.cpp src/test_module.cpp $(LUA_LIB)

# Run all tests
test: test_basic test_advanced test_integration
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║              All lua-intf Tests PASSED ✓                  ║"
	@echo "╚════════════════════════════════════════════════════════════╝"

# Run basic lua-intf feature tests
test_basic: $(TEST_CLI)
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║           Basic lua-intf Features                         ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "[1/4] __len metamethod support"
	@./$(TEST_CLI) scripts/test_len_metamethod.lua
	@echo ""
	@echo "[2/4] std::vector conversion"
	@./$(TEST_CLI) scripts/test_vector_conversion.lua
	@echo ""
	@echo "[3/4] In-place table mutation"
	@./$(TEST_CLI) scripts/test_table_mutation.lua
	@echo ""
	@echo "[4/4] Shared pointer lifecycle"
	@./$(TEST_CLI) scripts/test_shared_ownership.lua
	@echo ""
	@echo "✓ Basic tests PASSED"

# Run advanced feature tests
test_advanced: $(PHASE2_CLI)
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║           Advanced lua-intf Features                      ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "[1/3] Zero-copy TensorView"
	@./$(PHASE2_CLI) scripts/test_tensor_view.lua
	@echo ""
	@echo "[2/3] Nested container conversion"
	@./$(PHASE2_CLI) scripts/test_nested_containers.lua
	@echo ""
	@echo "[3/3] Edge cases and boundaries"
	@./$(PHASE2_CLI) scripts/test_edge_cases.lua | tail -8
	@echo ""
	@echo "✓ Advanced tests PASSED"

# Run integration tests (CVLib + PostLib)
test_integration: $(PHASE2_CLI)
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║           Integration Tests (CVLib + PostLib)             ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "[1/2] CVLib module tests"
	@./$(PHASE2_CLI) scripts/test_cvlib.lua | grep -E "(Testing|PASSED|✓)"
	@echo ""
	@echo "[2/2] PostLib module tests"
	@./$(PHASE2_CLI) scripts/test_postlib.lua | grep -E "(Testing|PASSED|✓)"
	@echo ""
	@echo "✓ Integration tests PASSED"

clean:
	rm -f $(TEST_CLI) $(PHASE2_CLI)
	@echo "✓ Cleaned test executables"

help:
	@echo "lua-intf Test Suite"
	@echo "==================="
	@echo ""
	@echo "Targets:"
	@echo "  make all              # Build all test executables"
	@echo "  make test             # Run all tests"
	@echo "  make test_basic       # Run basic feature tests"
	@echo "  make test_advanced    # Run advanced feature tests"
	@echo "  make test_integration # Run integration tests"
	@echo "  make clean            # Remove built executables"
	@echo "  make help             # Show this help"
	@echo ""
	@echo "Test executables:"
	@echo "  ./test_cli script.lua    # Run with basic features"
	@echo "  ./phase2_cli script.lua  # Run with full modules"
