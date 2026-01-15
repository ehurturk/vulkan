# VGE Test Suite

This directory contains the test suite for the Vulkan Game Engine (VGE).

## Structure

Tests are organized by engine subsystem:

```
test/
├── concurrency/        # Job system, threading tests
├── memory/            # Custom allocators (pool, stack, destack)
├── resource/          # Resource pool, handle management
└── CMakeLists.txt     # Auto-discovers all *_tests.cpp files
```

## Adding New Tests

To add a new test:

1. Create a new file following the naming convention `*_tests.cpp`
2. Place it in the appropriate subdirectory (or create a new one)
3. CMake will automatically discover and build it

**Example:**

```cpp
// test/resource/my_new_tests.cpp
#include <gtest/gtest.h>
#include <resource/my_feature.hpp>

TEST(MyFeature, DoesWhatIExpect) {
    // Your test here
    EXPECT_EQ(1 + 1, 2);
}
```

No need to modify `CMakeLists.txt` - it uses `file(GLOB_RECURSE)` to find all test files.

## Running Tests

Build and run all tests:
```bash
cmake --build build/Debug
cd build/Debug
ctest --output-on-failure
```

Or run the test binary directly:
```bash
./build/Debug/bin/EngineTests
```

### Running Specific Tests

GoogleTest supports filtering:

```bash
# Run only ResourcePool tests
./build/Debug/bin/EngineTests --gtest_filter="ResourcePoolTest.*"

# Run only allocation tests across all suites
./build/Debug/bin/EngineTests --gtest_filter="*Allocat*"

# Run everything except stress tests
./build/Debug/bin/EngineTests --gtest_filter="-*Stress*"
```

## Test Guidelines

1. **Use Test Fixtures** for setup/teardown:
   ```cpp
   class MyTest : public ::testing::Test {
   protected:
       void SetUp() override { /* setup */ }
       void TearDown() override { /* cleanup */ }
       MyClass obj;
   };
   ```

2. **Name tests descriptively**:
   - `TEST(SuiteName, WhatIsBeingTested)`
   - Examples: `FreeInvalidatesHandle`, `GenerationCounterIncrementsOnFree`

3. **One assertion per concept**:
   - Don't test 10 things in one `TEST()`
   - Split into multiple focused tests

4. **Use ASSERT vs EXPECT appropriately**:
   - `ASSERT_*` fails immediately (use for preconditions)
   - `EXPECT_*` continues after failure (use for actual tests)

## Dependencies

- **GoogleTest**: Fetched automatically via FetchContent
- **Engine**: Links against the `engine` static library
