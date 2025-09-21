#include <gtest/gtest.h>
#include <memory_resource>
#include <vector>
#include <list>
#include <unordered_map>
#include <thread>
#include <random>
#include <algorithm>

#include <core/memory/pool_allocator.hpp>
#include <core/stl/FixedPoolResource.hpp>

#include "core/stl/DestackMemoryResource.hpp"
#include "core/stl/MultipoolMemoryResource.hpp"

using namespace Core::Allocator;
using namespace Core::MemoryResource;

TEST(FixedPoolAllocator, BasicAllocateDeallocate) {
    constexpr std::size_t align = alignof(std::max_align_t);
    FixedPoolAllocator pool(64, align, 8);

    EXPECT_EQ(pool.block_size(), 64u);
    EXPECT_EQ(pool.block_align(), align);
    EXPECT_EQ(pool.capacity(), 8u);
    EXPECT_EQ(pool.in_use(), 0u);
    EXPECT_EQ(pool.free_blocks(), 8u);

    void* p1 = pool.allocate_block();
    void* p2 = pool.allocate_block();
    void* p3 = pool.allocate_block();

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    EXPECT_TRUE(Core::MemoryUtil::IsAligned(p1, align));
    EXPECT_TRUE(Core::MemoryUtil::IsAligned(p2, align));
    EXPECT_TRUE(Core::MemoryUtil::IsAligned(p3, align));

    EXPECT_EQ(pool.in_use(), 3u);
    EXPECT_EQ(pool.free_blocks(), 5u);
    EXPECT_TRUE(pool.owns(p1));
    EXPECT_TRUE(pool.owns(p2));
    EXPECT_TRUE(pool.owns(p3));

    pool.deallocate_block(p2);
    pool.deallocate_block(p3);
    pool.deallocate_block(p1);

    EXPECT_EQ(pool.in_use(), 0u);
    EXPECT_EQ(pool.free_blocks(), 8u);
}

TEST(FixedPoolAllocator, ExhaustsAndRecovers) {
    FixedPoolAllocator pool(64, alignof(std::max_align_t), 2);

    void* p1 = pool.allocate_block();
    void* p2 = pool.allocate_block();
    EXPECT_EQ(pool.in_use(), 2u);

    EXPECT_EQ(pool.allocate_block(), nullptr);

    pool.deallocate_block(p2);

    EXPECT_NE(pool.allocate_block(), nullptr);

    pool.deallocate_block(p1);

    EXPECT_EQ(pool.in_use(), 1u);
}

TEST(FixedPoolAllocator, AlignmentHonored) {
    constexpr std::size_t align = 64;  // over-aligned
    FixedPoolAllocator pool(/*block_size=*/96, /*block_align=*/align, /*blocks=*/4);

    void* p = pool.allocate_block();
    ASSERT_NE(p, nullptr);
    EXPECT_TRUE(Core::MemoryUtil::IsAligned(p, align));
    pool.deallocate_block(p);
}

// ============================================================================
// 2) Using pool as a classic STL allocator (e.g., std::list)
//    We define a tiny adapter inside the test that binds to the same pool.
// ============================================================================
namespace {
template <class U>
struct PoolAdapter {
    using value_type = U;
    FixedPoolAllocator* pool = nullptr;

    PoolAdapter() = default;
    explicit PoolAdapter(FixedPoolAllocator& p) : pool(&p) {}

    template <class V>
    PoolAdapter(const PoolAdapter<V>& o) noexcept : pool(o.pool) {}

    U* allocate(std::size_t n) {
        // Node containers allocate one object at a time
        if (n != 1)
            throw std::bad_alloc();
        void* p = pool->allocate_block();
        if (!p)
            throw std::bad_alloc();
        // alignment of pool must be >= alignof(U)
        return static_cast<U*>(p);
    }
    void deallocate(U* p, std::size_t) noexcept { pool->deallocate_block(p); }

    template <class V>
    struct rebind {
        using other = PoolAdapter<V>;
    };

    bool operator==(const PoolAdapter& rhs) const noexcept { return pool == rhs.pool; }
    bool operator!=(const PoolAdapter& rhs) const noexcept { return !(*this == rhs); }
};
}  // namespace

TEST(FixedPoolAllocator_STL, StdListUsesPool) {
    FixedPoolAllocator pool(/*block_size=*/256, /*block_align=*/alignof(std::max_align_t),
                            /*blocks=*/1024);

    using T = int;
    using Alloc = PoolAdapter<T>;
    std::list<T, Alloc> lst{Alloc(pool)};

    const std::size_t before_in_use = pool.in_use();

    for (int i = 0; i < 100; ++i)
        lst.push_back(i);
    EXPECT_EQ(lst.size(), 100u);
    EXPECT_GE(pool.in_use(), before_in_use + 100);

    lst.clear();
    EXPECT_EQ(pool.in_use(), before_in_use);
}

TEST(FixedPoolAllocator_PMR, PmrListAllocatesFromPool) {
    FixedPoolAllocator pool(/*block_size=*/256, /*block_align=*/alignof(std::max_align_t),
                            /*blocks=*/2048);
    FixedPoolResource resource(pool);

    std::pmr::list<int> lst{&resource};

    const std::size_t before = pool.in_use();
    for (int i = 0; i < 200; ++i)
        lst.push_back(i);
    EXPECT_EQ(lst.size(), 200u);
    EXPECT_GT(pool.in_use(), before);

    lst.clear();
    EXPECT_EQ(pool.in_use(), before);
}

TEST(FixedPoolAllocator_PMR, ThrowsWhenExhausted) {
    FixedPoolAllocator pool(/*block_size=*/sizeof(int), /*block_align=*/alignof(int),
                            /*blocks=*/8);
    FixedPoolResource resource(pool);

    std::pmr::list<int> lst{&resource};

    for (int i = 0; i < 8; ++i)
        lst.push_back(i);
    EXPECT_EQ(pool.free_blocks(), 0u);

    EXPECT_THROW({ lst.push_back(9); }, std::bad_alloc);

    lst.pop_back();
    EXPECT_NO_THROW({ lst.push_back(10); });
}

class PMRWrapperMultipoolTest : public ::testing::Test {};

TEST_F(PMRWrapperMultipoolTest, MultipoolWorksWithSTL) {
    MultipoolMemoryResource multipool(16, 256, 100);

    // served by the 32 byte pool
    std::pmr::list<int> my_list(&multipool);
    my_list.push_back(1);
    my_list.push_back(2);

    // will request 40 bytes, use the 64 byte pool
    std::pmr::string my_string("Hello, this is a relatively long string.", &multipool);

    // 1024 byte request is too large by any pool, should allocate with upstream allocator
    std::pmr::vector<char> large_vector(1024, &multipool);
    std::cout << "Successfully allocated from various pools and the upstream resource."
              << std::endl;
    // container destructors will call do_deallocate
}