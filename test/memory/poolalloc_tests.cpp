#include <gtest/gtest.h>

#include <core/memory/pool_allocator.hpp>

using namespace Core;

struct Object {
    int x;
    int y;
    int z;
    double vx;
    double vz;
};

class PoolAllocatorTest : public ::testing::Test {
  protected:
    void SetUp() override { allocator = makePoolAllocator<Object>(1, true); }

    void TearDown() override { allocator.reset(); }

    std::unique_ptr<PoolAllocator<Object>> allocator;
};

TEST_F(PoolAllocatorTest, Constructor_InitializesCorrectly) {
    EXPECT_EQ(allocator->capacity(), 1);
    EXPECT_EQ(allocator->allocated(), 0);
    EXPECT_EQ(allocator->available(), 1);
}
