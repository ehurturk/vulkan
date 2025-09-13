#include <gtest/gtest.h>
#include <core/memory/stack_allocator.hpp>
#include <core/memory/memory.hpp>
#include <core/logger.hpp>
#include <iostream>
using namespace Core;

class StackAllocatorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create a 1KB allocator for testing
        allocator = std::make_unique<StackAllocator>(1024);
    }

    void TearDown() override { allocator.reset(); }

    std::unique_ptr<StackAllocator> allocator;
};

TEST_F(StackAllocatorTest, Constructor_InitializesCorrectly) {
    EXPECT_EQ(allocator->getUsedBytes(), 0);
    EXPECT_EQ(allocator->getAvailableBytes(), 1024);
    EXPECT_EQ(allocator->getMarker(), 0);
}

TEST_F(StackAllocatorTest, BasicAllocation_ReturnsValidPointer) {
    void *ptr = allocator->alloc(64, MemoryTag::MEMORY_TAG_APPLICATION);

    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(allocator->getUsedBytes(), 64);
    EXPECT_EQ(allocator->getAvailableBytes(), 1024 - 64);
}

TEST_F(StackAllocatorTest, MultipleAllocations_StackCorrectly) {
    void *ptr1 = allocator->alloc(32, MemoryTag::MEMORY_TAG_APPLICATION);
    void *ptr2 = allocator->alloc(48, MemoryTag::MEMORY_TAG_ARRAY);
    void *ptr3 = allocator->alloc(16, MemoryTag::MEMORY_TAG_BST);

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);

    // emulate stack behaviour - ptrs should be ascending (stack grows up)
    EXPECT_LT(ptr1, ptr2);
    EXPECT_LT(ptr2, ptr3);

    EXPECT_EQ(allocator->getUsedBytes(), 32 + 48 + 16);
}

// Alignment tests
TEST_F(StackAllocatorTest, Alignment_16ByteDefault) {
    void *ptr1 = allocator->alloc(1, MemoryTag::MEMORY_TAG_APPLICATION); // Should align to 16
    void *ptr2 = allocator->alloc(1, MemoryTag::MEMORY_TAG_APPLICATION); // Should align to next 16

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);

    EXPECT_EQ(addr1 % 16, 0);     // 16-byte aligned
    EXPECT_EQ(addr2 % 16, 0);     // 16-byte aligned
    EXPECT_EQ(addr2 - addr1, 16); // Exactly 16 bytes apart
}

TEST_F(StackAllocatorTest, Alignment_CustomAlignment) {
    void *ptr1 = allocator->alloc(1, MemoryTag::MEMORY_TAG_APPLICATION, 32); // 32-byte alignment
    void *ptr2 = allocator->alloc(1, MemoryTag::MEMORY_TAG_APPLICATION, 64); // 64-byte alignment

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);

    EXPECT_EQ(addr1 % 32, 0); // 32-byte aligned
    EXPECT_EQ(addr2 % 64, 0); // 64-byte aligned
}

TEST_F(StackAllocatorTest, Alignment_PowerOfTwoOnly) {
    for (u32 alignment = 1; alignment <= 256; alignment *= 2) {
        StackAllocator testAlloc(1024);
        void *ptr = testAlloc.alloc(1, MemoryTag::MEMORY_TAG_APPLICATION, alignment);

        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        EXPECT_EQ(addr % alignment, 0) << "Failed alignment test for " << alignment << " bytes";
    }
}

TEST_F(StackAllocatorTest, Marker_TracksCurrentPosition) {
    auto marker1 = allocator->getMarker();
    EXPECT_EQ(marker1, 0);

    allocator->alloc(64, MemoryTag::MEMORY_TAG_APPLICATION);
    auto marker2 = allocator->getMarker();
    EXPECT_EQ(marker2, 64);

    allocator->alloc(32, MemoryTag::MEMORY_TAG_ARRAY);
    auto marker3 = allocator->getMarker();
    EXPECT_EQ(marker3, 64 + 32);
}

TEST_F(StackAllocatorTest, FreeTo_ResetsToMarker) {
    void *ptr1 = allocator->alloc(64, MemoryTag::MEMORY_TAG_APPLICATION);
    auto marker = allocator->getMarker();

    void *ptr2 = allocator->alloc(32, MemoryTag::MEMORY_TAG_ARRAY);
    EXPECT_EQ(allocator->getUsedBytes(), 64 + 32);

    allocator->freeTo(marker);
    EXPECT_EQ(allocator->getUsedBytes(), 64);
    EXPECT_EQ(allocator->getMarker(), marker);

    void *ptr3 = allocator->alloc(48, MemoryTag::MEMORY_TAG_BST);
    EXPECT_NE(ptr3, nullptr);
}

TEST_F(StackAllocatorTest, Clear_ResetsToEmpty) {
    allocator->alloc(100, MemoryTag::MEMORY_TAG_APPLICATION);
    allocator->alloc(200, MemoryTag::MEMORY_TAG_ARRAY);

    EXPECT_GT(allocator->getUsedBytes(), 0);

    allocator->clear();

    EXPECT_EQ(allocator->getUsedBytes(), 0);
    EXPECT_EQ(allocator->getAvailableBytes(), 1024);
    EXPECT_EQ(allocator->getMarker(), 0);
}

TEST_F(StackAllocatorTest, OutOfMemory_ReturnsNull) {
    // Try to allocate more than available
    void *ptr = allocator->alloc(2048, MemoryTag::MEMORY_TAG_APPLICATION); // 2KB from 1KB allocator
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(allocator->getUsedBytes(), 0); // Nothing should be allocated
}

TEST_F(StackAllocatorTest, ExactFit_WorksCorrectly) {
    void *ptr = allocator->alloc(1024, MemoryTag::MEMORY_TAG_APPLICATION);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(allocator->getUsedBytes(), 1024);
    EXPECT_EQ(allocator->getAvailableBytes(), 0);

    // Next allocation should fail
    void *ptr2 = allocator->alloc(1, MemoryTag::MEMORY_TAG_ARRAY);
    EXPECT_EQ(ptr2, nullptr);
}

TEST_F(StackAllocatorTest, ZeroSizeAllocation_HandledCorrectly) {
    void *ptr = allocator->alloc(0, MemoryTag::MEMORY_TAG_APPLICATION);

    // Implementation might return nullptr or valid pointer - both are acceptable
    // But used bytes should remain 0 or minimal alignment
    EXPECT_LE(allocator->getUsedBytes(), 16); // At most one alignment worth
}

TEST_F(StackAllocatorTest, AlignmentWithLargeValues_DoesNotOverflow) {
    // Test alignment near the buffer boundary
    allocator->alloc(1000, MemoryTag::MEMORY_TAG_APPLICATION); // Use most of buffer

    // Try to allocate with large alignment - should fail gracefully
    void *ptr = allocator->alloc(8, MemoryTag::MEMORY_TAG_ARRAY, 64);
    EXPECT_EQ(ptr, nullptr);
}

// Memory access tests (writing/reading from allocated memory)
TEST_F(StackAllocatorTest, AllocatedMemory_IsWritableAndReadable) {
    struct TestData {
        int value1;
        float value2;
        char value3;
    };

    TestData *data = static_cast<TestData *>(
        allocator->alloc(sizeof(TestData), MemoryTag::MEMORY_TAG_APPLICATION));
    ASSERT_NE(data, nullptr);

    // Write to allocated memory
    data->value1 = 42;
    data->value2 = 3.14f;
    data->value3 = 'A';

    // Read back and verify
    EXPECT_EQ(data->value1, 42);
    EXPECT_FLOAT_EQ(data->value2, 3.14f);
    EXPECT_EQ(data->value3, 'A');
}

TEST_F(StackAllocatorTest, MultipleAllocations_DontOverlap) {
    const int numInts = 10;
    int *array1 = static_cast<int *>(
        allocator->alloc(sizeof(int) * numInts, MemoryTag::MEMORY_TAG_APPLICATION));
    int *array2 =
        static_cast<int *>(allocator->alloc(sizeof(int) * numInts, MemoryTag::MEMORY_TAG_ARRAY));

    ASSERT_NE(array1, nullptr);
    ASSERT_NE(array2, nullptr);

    // Fill arrays with different patterns
    for (int i = 0; i < numInts; ++i) {
        array1[i] = i;
        array2[i] = i * 2;
    }

    // Verify arrays maintain their values (no overlap)
    for (int i = 0; i < numInts; ++i) {
        EXPECT_EQ(array1[i], i);
        EXPECT_EQ(array2[i], i * 2);
    }
}

TEST_F(StackAllocatorTest, Allocation_IsConstantTime) {
    const int numAllocations = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numAllocations && allocator->getAvailableBytes() > 0; ++i) {
        void *ptr = allocator->alloc(1, MemoryTag::MEMORY_TAG_APPLICATION, 1);
        EXPECT_NE(ptr, nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Completed " << numAllocations << " allocations in " << duration.count() << " ms."
              << std::endl;
    // Should be very fast - less than 10ms for 1000 allocations
    EXPECT_LT(duration.count(), 1000);
}
