#include <gtest/gtest.h>
#include <core/memory/stack_allocator.hpp>
#include <core/stl/StackMemoryResource.hpp>
#include <core/memory/memory.hpp>
#include <core/logger.hpp>
#include <iostream>
using namespace Core::Allocator;
using namespace Core::MemoryResource;

class StackAllocatorTest : public ::testing::Test {
   protected:
    StackAllocator allocator{1024};
};

TEST_F(StackAllocatorTest, HandlesBasicAllocation) {
    ASSERT_EQ(allocator.getUsedBytes(), 0);
    void* p1 = allocator.alloc(100);
    ASSERT_NE(p1, nullptr);
    ASSERT_GE(allocator.getUsedBytes(), 100);
}

TEST_F(StackAllocatorTest, RespectsAlignment) {
    void* p1 = allocator.alloc(1, 16);
    ASSERT_TRUE(Core::MemoryUtil::IsAligned(p1, 16));

    void* p2 = allocator.alloc(1, 32);
    ASSERT_TRUE(Core::MemoryUtil::IsAligned(p2, 32));

    void* p3 = allocator.alloc(1, 64);
    ASSERT_TRUE(Core::MemoryUtil::IsAligned(p3, 64));
}

TEST_F(StackAllocatorTest, FreeToMarkerResetsState) {
    auto initial_marker = allocator.getMarker();
    void* a = allocator.alloc(50);
    auto marker_after_50 = allocator.getMarker();
    void* b = allocator.alloc(100);

    allocator.freeTo(marker_after_50);
    ASSERT_EQ(allocator.getMarker(), marker_after_50);

    allocator.freeTo(initial_marker);
    ASSERT_EQ(allocator.getUsedBytes(), 0);
}

TEST_F(StackAllocatorTest, ThrowsExceptionWhenFull) {
    void* c = allocator.alloc(1000);
    ASSERT_THROW(allocator.alloc(100), std::bad_alloc);
}

TEST_F(StackAllocatorTest, ClearResetsAllocator) {
    void* d = allocator.alloc(200);
    allocator.clear();
    ASSERT_EQ(allocator.getUsedBytes(), 0);
}

class PMRWrapperTest : public ::testing::Test {};

TEST_F(PMRWrapperTest, StackPMRCanBeUsedByVector) {
    StackAllocator allocator(1024);
    StackMemoryResource resource(allocator);
    std::pmr::vector<int> vec(&resource);

    vec.push_back(1);
    ASSERT_GT(allocator.getUsedBytes(), 0);
}

TEST_F(PMRWrapperTest, DeallocationIsANoOp) {
    StackAllocator allocator(1024);
    StackMemoryResource resource(allocator);
    std::pmr::vector<int> vec({1, 2, 3, 4}, &resource);
    auto bytes_before = allocator.getUsedBytes();
    vec.clear();
    auto bytes_after = allocator.getUsedBytes();
    ASSERT_EQ(bytes_before, bytes_after);
}

TEST_F(PMRWrapperTest, ThrowsBadAllocWhenAllocatorIsFull) {
    StackAllocator allocator(64);
    StackMemoryResource resource(allocator);
    std::pmr::vector<long long> vec(&resource);

    vec.reserve(8);

    for (int i = 0; i < 8; ++i) {
        vec.push_back(static_cast<long long>(i));
    }

    LOG_INFO("Amount left: {}", allocator.getAvailableBytes());

    ASSERT_THROW(vec.push_back(2), std::bad_alloc);
}