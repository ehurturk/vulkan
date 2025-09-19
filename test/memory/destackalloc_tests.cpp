#include <gtest/gtest.h>
#include <core/memory/destack_allocator.hpp>
#include <core/memory/memory.hpp>
#include <cstring>

#include "core/logger.hpp"

namespace Core {

class DestackAllocatorTest : public ::testing::Test {
  protected:
    void SetUp() override { allocator = std::make_unique<DestackAllocator>(1024); }

    void TearDown() override { allocator.reset(); }

    std::unique_ptr<DestackAllocator> allocator;
};

TEST_F(DestackAllocatorTest, Constructor_InitializesCorrectly) {
    EXPECT_EQ(allocator->getUsedBytes(), 0);
    EXPECT_EQ(allocator->getAvailableBytes(), 1024);
}

TEST_F(DestackAllocatorTest, TopAllocation_ReturnsValidPointer) {
    void *ptr = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_TOP,
                                 MemoryTag::MEMORY_TAG_ARRAY);

    EXPECT_NE(ptr, nullptr);
    EXPECT_GT(allocator->getUsedBytes(), 0);
    EXPECT_LT(allocator->getAvailableBytes(), 1024);
}

TEST_F(DestackAllocatorTest, BottomAllocation_ReturnsValidPointer) {
    void *ptr = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                 MemoryTag::MEMORY_TAG_ARRAY);

    EXPECT_NE(ptr, nullptr);
    EXPECT_GT(allocator->getUsedBytes(), 0);
    EXPECT_LT(allocator->getAvailableBytes(), 1024);
}

TEST_F(DestackAllocatorTest, BothDirections_AllocateCorrectly) {
    void *topPtr = allocator->alloc(32, DestackAllocator::HeapDirection::FRAME_TOP,
                                    MemoryTag::MEMORY_TAG_ARRAY);
    void *bottomPtr = allocator->alloc(48, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                       MemoryTag::MEMORY_TAG_DARRAY);

    EXPECT_NE(topPtr, nullptr);
    EXPECT_NE(bottomPtr, nullptr);
    std::cout << "TOP PTR: " << topPtr << "| BOTTOM PTR: " << bottomPtr << std::endl;
    // top ptr:    0x130012400
    // bottom ptr: 0x130012000
    // diff: 0x400 -> a clear difference of 0x400 bytes (with alignment)
    // Top pointer should be at lower address than bottom pointer
    // TODO: 0X400 difference between top and bottom (256*4 = 1024) byte difference.
    EXPECT_LT(bottomPtr, topPtr);

    // Should have allocated both sizes
    U32 expectedUsed = 32 + 48;
    // Account for potential alignment padding
    // TODO: actual: 48 vs 80
    EXPECT_GE(allocator->getUsedBytes(), expectedUsed);
}

// Alignment tests
TEST_F(DestackAllocatorTest, TopAlignment_16ByteDefault) {
    void *ptr1 = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    void *ptr2 = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_ARRAY);

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);

    EXPECT_EQ(addr1 % 16, 0); // 16-byte aligned
    EXPECT_EQ(addr2 % 16, 0); // 16-byte aligned
}

TEST_F(DestackAllocatorTest, BottomAlignment_16ByteDefault) {
    void *ptr1 = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    void *ptr2 = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_ARRAY);

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);

    EXPECT_EQ(addr1 % 16, 0); // 16-byte aligned
    EXPECT_EQ(addr2 % 16, 0); // 16-byte aligned
}

TEST_F(DestackAllocatorTest, CustomAlignment_WorksCorrectly) {
    void *topPtr = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_TOP,
                                    MemoryTag::MEMORY_TAG_ARRAY, 32);
    void *bottomPtr = allocator->alloc(1, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                       MemoryTag::MEMORY_TAG_ARRAY, 64);

    uintptr_t topAddr = reinterpret_cast<uintptr_t>(topPtr);
    uintptr_t bottomAddr = reinterpret_cast<uintptr_t>(bottomPtr);

    EXPECT_EQ(topAddr % 32, 0);    // 32-byte aligned
    EXPECT_EQ(bottomAddr % 64, 0); // 64-byte aligned
}

// Marker and freeTo tests
TEST_F(DestackAllocatorTest, TopMarker_TracksPosition) {
    auto initialMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_TOP);

    allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_TOP, MemoryTag::MEMORY_TAG_ARRAY);
    auto afterAllocMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_TOP);

    EXPECT_NE(initialMarker.mark, afterAllocMarker.mark);
    EXPECT_EQ(initialMarker.dir, DestackAllocator::HeapDirection::FRAME_TOP);
    EXPECT_EQ(afterAllocMarker.dir, DestackAllocator::HeapDirection::FRAME_TOP);
}

TEST_F(DestackAllocatorTest, BottomMarker_TracksPosition) {
    auto initialMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_BOTTOM);

    allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                     MemoryTag::MEMORY_TAG_ARRAY);
    auto afterAllocMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_BOTTOM);

    EXPECT_NE(initialMarker.mark, afterAllocMarker.mark);
    EXPECT_EQ(initialMarker.dir, DestackAllocator::HeapDirection::FRAME_BOTTOM);
    EXPECT_EQ(afterAllocMarker.dir, DestackAllocator::HeapDirection::FRAME_BOTTOM);
}

TEST_F(DestackAllocatorTest, FreeToTopMarker_ResetsCorrectly) {
    void *ptr1 = allocator->alloc(32, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    auto marker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_TOP);

    void *ptr2 = allocator->alloc(48, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_DARRAY);
    U32 usedAfterSecondAlloc = allocator->getUsedBytes();

    allocator->freeTo(marker);
    U32 usedAfterFree = allocator->getUsedBytes();
    // TODO: actual: 0 vs 0
    EXPECT_LT(usedAfterFree, usedAfterSecondAlloc);

    // Should be able to allocate again from the freed space
    void *ptr3 = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_DICT);
    EXPECT_NE(ptr3, nullptr);
}

TEST_F(DestackAllocatorTest, FreeToBottomMarker_ResetsCorrectly) {
    void *ptr1 = allocator->alloc(32, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    auto marker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_BOTTOM);

    void *ptr2 = allocator->alloc(48, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_DARRAY);
    U32 usedAfterSecondAlloc = allocator->getUsedBytes();

    allocator->freeTo(marker);
    U32 usedAfterFree = allocator->getUsedBytes();

    EXPECT_LT(usedAfterFree, usedAfterSecondAlloc);

    // Should be able to allocate again from the freed space
    void *ptr3 = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_DICT);
    EXPECT_NE(ptr3, nullptr);
}

TEST_F(DestackAllocatorTest, Clear_ResetsToEmpty) {
    allocator->alloc(100, DestackAllocator::HeapDirection::FRAME_TOP, MemoryTag::MEMORY_TAG_ARRAY);
    allocator->alloc(200, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                     MemoryTag::MEMORY_TAG_DARRAY);
    EXPECT_GT(allocator->getUsedBytes(), 0);

    allocator->clear();

    EXPECT_EQ(allocator->getUsedBytes(), 0);
    EXPECT_EQ(allocator->getAvailableBytes(), 1024);
}

// Collision and boundary tests
TEST_F(DestackAllocatorTest, AllocationsFromBothEnds_DontOverlap) {
    // Fill from both ends and ensure they don't collide
    std::vector<void *> topPtrs;
    std::vector<void *> bottomPtrs;

    constexpr U32 allocSize = 32;

    // Allocate from both ends alternately
    for (int i = 0; i < 10 && allocator->getAvailableBytes() >= allocSize * 2; ++i) {
        // allocate from the frame top:
        // [          ]
        // b          t
        // [       |  ]
        // b       t
        void *topPtr = allocator->alloc(allocSize, DestackAllocator::HeapDirection::FRAME_TOP,
                                        MemoryTag::MEMORY_TAG_ARRAY);
        // allocate from the frame bottom:
        // [          ]
        // b          t
        // [  |    |  ]
        //    b    t
        void *bottomPtr = allocator->alloc(allocSize, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                           MemoryTag::MEMORY_TAG_DARRAY);
        // TODO: in the end, top should be greater than bottom obviously.
        if (topPtr != nullptr)
            topPtrs.push_back(topPtr);
        if (bottomPtr != nullptr)
            bottomPtrs.push_back(bottomPtr);

        std::cout << "TOP: " << topPtr << " | BOTTOM: " << bottomPtr << std::endl;

        if (topPtr && bottomPtr) {
            EXPECT_LT(bottomPtr, topPtr);
        }
    }

    EXPECT_GT(topPtrs.size(), 0);
    EXPECT_GT(bottomPtrs.size(), 0);
}

TEST_F(DestackAllocatorTest, ExhaustMemory_HandledGracefully) {
    // Try to allocate more memory than available
    const U32 largeSize = 512;

    void *ptr1 = allocator->alloc(largeSize, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    void *ptr2 = allocator->alloc(largeSize, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                  MemoryTag::MEMORY_TAG_DARRAY);

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);

    // This should fail - not enough space
    void *ptr3 = allocator->alloc(largeSize, DestackAllocator::HeapDirection::FRAME_TOP,
                                  MemoryTag::MEMORY_TAG_ARRAY);
    EXPECT_EQ(ptr3, nullptr);
}

TEST_F(DestackAllocatorTest, ZeroSizeAllocation_HandledCorrectly) {
    void *topPtr = allocator->alloc(0, DestackAllocator::HeapDirection::FRAME_TOP,
                                    MemoryTag::MEMORY_TAG_ARRAY);
    void *bottomPtr = allocator->alloc(0, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                       MemoryTag::MEMORY_TAG_DARRAY);

    // Implementation might return nullptr or valid pointer for zero size
    // But should not crash or corrupt state
    U32 usedAfter = allocator->getUsedBytes();
    EXPECT_LE(usedAfter, 32); // At most alignment padding
}

// Memory access and data integrity tests
TEST_F(DestackAllocatorTest, AllocatedMemory_IsWritableAndReadable) {
    struct TestData {
        int value1;
        float value2;
        char value3[16];
    };

    TestData *topData = static_cast<TestData *>(allocator->alloc(
        sizeof(TestData), DestackAllocator::HeapDirection::FRAME_TOP, MemoryTag::MEMORY_TAG_ARRAY));
    TestData *bottomData = static_cast<TestData *>(
        allocator->alloc(sizeof(TestData), DestackAllocator::HeapDirection::FRAME_BOTTOM,
                         MemoryTag::MEMORY_TAG_DARRAY));

    ASSERT_NE(topData, nullptr);
    ASSERT_NE(bottomData, nullptr);

    // Write to allocated memory
    topData->value1 = 42;
    topData->value2 = 3.14f;
    strcpy(topData->value3, "TOP");

    bottomData->value1 = 84;
    bottomData->value2 = 2.71f;
    strcpy(bottomData->value3, "BOTTOM");

    // Read back and verify
    EXPECT_EQ(topData->value1, 42);
    EXPECT_FLOAT_EQ(topData->value2, 3.14f);
    EXPECT_STREQ(topData->value3, "TOP");

    EXPECT_EQ(bottomData->value1, 84);
    EXPECT_FLOAT_EQ(bottomData->value2, 2.71f);
    EXPECT_STREQ(bottomData->value3, "BOTTOM");
}

TEST_F(DestackAllocatorTest, MultipleAllocations_DontOverlap) {
    const int arraySize = 10;

    int *topArray = static_cast<int *>(allocator->alloc(sizeof(int) * arraySize,
                                                        DestackAllocator::HeapDirection::FRAME_TOP,
                                                        MemoryTag::MEMORY_TAG_ARRAY));
    int *bottomArray = static_cast<int *>(
        allocator->alloc(sizeof(int) * arraySize, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                         MemoryTag::MEMORY_TAG_DARRAY));

    ASSERT_NE(topArray, nullptr);
    ASSERT_NE(bottomArray, nullptr);

    // Fill arrays with different patterns
    for (int i = 0; i < arraySize; ++i) {
        topArray[i] = i;
        bottomArray[i] = i * 2;
    }

    // Verify arrays maintain their values (no overlap)
    for (int i = 0; i < arraySize; ++i) {
        EXPECT_EQ(topArray[i], i);
        EXPECT_EQ(bottomArray[i], i * 2);
    }
}

// Complex scenario tests
TEST_F(DestackAllocatorTest, MixedOperations_WorkCorrectly) {
    // Simulate typical usage: allocate from both ends, get markers, free some, allocate more

    // Initial allocations
    void *topPtr1 = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_TOP,
                                     MemoryTag::MEMORY_TAG_GAME);
    void *bottomPtr1 = allocator->alloc(64, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                        MemoryTag::MEMORY_TAG_ARRAY);

    EXPECT_NE(topPtr1, nullptr);
    EXPECT_NE(bottomPtr1, nullptr);

    // Get markers
    auto topMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_TOP);
    auto bottomMarker = allocator->getMarker(DestackAllocator::HeapDirection::FRAME_BOTTOM);

    // More allocations
    void *topPtr2 = allocator->alloc(32, DestackAllocator::HeapDirection::FRAME_TOP,
                                     MemoryTag::MEMORY_TAG_DICT);
    void *bottomPtr2 = allocator->alloc(32, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                        MemoryTag::MEMORY_TAG_DICT);

    EXPECT_NE(topPtr2, nullptr);
    EXPECT_NE(bottomPtr2, nullptr);

    U32 usedBeforeFree = allocator->getUsedBytes();

    // Free back to markers
    allocator->freeTo(topMarker);
    allocator->freeTo(bottomMarker);

    U32 usedAfterFree = allocator->getUsedBytes();
    EXPECT_LT(usedAfterFree, usedBeforeFree);

    // Should be able to allocate again
    void *topPtr3 = allocator->alloc(48, DestackAllocator::HeapDirection::FRAME_TOP,
                                     MemoryTag::MEMORY_TAG_DARRAY);
    void *bottomPtr3 = allocator->alloc(48, DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                        MemoryTag::MEMORY_TAG_DARRAY);

    EXPECT_NE(topPtr3, nullptr);
    EXPECT_NE(bottomPtr3, nullptr);
}
// TODO: disable performance test with debug builds for now
// Performance test (with realistic expectations)
 TEST_F(DestackAllocatorTest, Allocation_IsEfficient) {
     const int numAllocations = 100; // Reduced number for realistic test
     const U32 allocSize = 8;        // Small allocations

     auto start = std::chrono::high_resolution_clock::now();

     int successfulAllocs = 0;
     for (int i = 0; i < numAllocations && allocator->getAvailableBytes() >= allocSize * 2; ++i) {
         // Alternate between top and bottom
         auto direction = (i % 2 == 0) ? DestackAllocator::HeapDirection::FRAME_TOP
                                       : DestackAllocator::HeapDirection::FRAME_BOTTOM;

         void *ptr = allocator->alloc(allocSize, direction, MemoryTag::MEMORY_TAG_ARRAY,
                                      1); // 1-byte alignment
         if (ptr != nullptr) {
             successfulAllocs++;
         } else {
             break;
         }
     }

     auto end = std::chrono::high_resolution_clock::now();
     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

     EXPECT_GT(successfulAllocs, 0);
     EXPECT_LT(duration.count(), 1000); // 1ms

     std::cout << "Completed " << successfulAllocs << " allocations in " << duration.count()
               << " microseconds" << std::endl;
 }

} // namespace Core
