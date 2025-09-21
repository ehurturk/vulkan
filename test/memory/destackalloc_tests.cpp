#include <gtest/gtest.h>
#include <core/memory/destack_allocator.hpp>
#include <core/memory/memory.hpp>
#include <cstring>

#include "core/logger.hpp"

using namespace Core::Allocator;

class DestackAllocatorTest : public ::testing::Test {
   protected:
    DestackAllocator allocator{1024};
};

TEST_F(DestackAllocatorTest, AllocatesFromBottomAndTop) {
    void* p_bottom = allocator.alloc(100, DestackAllocator::HeapDirection::FRAME_BOTTOM);
    ASSERT_NE(p_bottom, nullptr);
    auto bytes_after_bottom = allocator.getUsedBytes();
    ASSERT_GE(bytes_after_bottom, 100);

    void* p_top = allocator.alloc(100, DestackAllocator::HeapDirection::FRAME_TOP);
    ASSERT_NE(p_top, nullptr);
    ASSERT_GT(allocator.getUsedBytes(), bytes_after_bottom);
}

TEST_F(DestackAllocatorTest, RespectsAlignmentFromBothEnds) {
    void* p_bottom = allocator.alloc(1, DestackAllocator::HeapDirection::FRAME_BOTTOM, 32);
    ASSERT_TRUE(Core::MemoryUtil::IsAligned(p_bottom, 32));

    void* p_top = allocator.alloc(1, DestackAllocator::HeapDirection::FRAME_TOP, 64);
    ASSERT_TRUE(Core::MemoryUtil::IsAligned(p_top, 64));
}

TEST_F(DestackAllocatorTest, ReturnsNullOnCollision) {
    allocator.alloc(500, DestackAllocator::HeapDirection::FRAME_BOTTOM);
    allocator.alloc(500, DestackAllocator::HeapDirection::FRAME_TOP);

    void* p_fail = allocator.alloc(50, DestackAllocator::HeapDirection::FRAME_BOTTOM);
    ASSERT_EQ(p_fail, nullptr);
}