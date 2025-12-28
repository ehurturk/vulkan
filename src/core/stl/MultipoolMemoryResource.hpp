#pragma once

#include <vector>
#include <memory>
#include <memory_resource>

#include "core/assert.hpp"
#include "core/memory/pool_allocator.hpp"

namespace Core::MemoryResource {

class MultipoolMemoryResource : public std::pmr::memory_resource {
   public:
    /**
     * @param min_block_size The size of the smallest pool's blocks. Must be a power of two.
     * @param max_block_size The size of the largest pool's blocks. Must be a power of two.
     * @param blocks_per_pool How many blocks each internal pool should contain.
     * @param upstream The memory resource to use for allocations larger than max_block_size.
     */
    MultipoolMemoryResource(size_t min_block_size,
                            size_t max_block_size,
                            size_t blocks_per_pool,
                            std::pmr::memory_resource* upstream = std::pmr::new_delete_resource())
        : m_Upstream(upstream) {
        ASSERT_MSG(min_block_size && (min_block_size & (min_block_size - 1)) == 0,
                   "Min block size must be a power of two.");
        ASSERT_MSG(max_block_size && (max_block_size & (max_block_size - 1)) == 0,
                   "Max block size must be a power of two.");

        for (size_t size = min_block_size; size <= max_block_size; size *= 2) {
            m_Pools.push_back(std::make_unique<Allocator::FixedPoolAllocator>(
                size, alignof(std::max_align_t), blocks_per_pool));
        }
    }

   private:
    std::vector<std::unique_ptr<Allocator::FixedPoolAllocator>> m_Pools;
    std::pmr::memory_resource* m_Upstream;

    void* do_allocate(size_t bytes, size_t alignment) override {
        for (const auto& pool : m_Pools) {
            if (bytes <= pool->block_size() && alignment <= pool->block_align()) {
                CORE_LOG_INFO("[MultipoolMemoryResource]: Allocating from pool with size {}.",
                              pool->block_size());
                void* p = pool->try_allocate_block();
                if (p) {
                    return p;
                }
            }
        }

        CORE_LOG_INFO("[MultipoolMemoryResource]: Allocating from upstream.");
        return m_Upstream->allocate(bytes, alignment);
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        for (const auto& pool : m_Pools) {
            if (pool->owns(p)) {
                CORE_LOG_INFO("[MultipoolMemoryResource]: Deallocating from pool.");
                pool->deallocate_block(p);
                return;
            }
        }
        CORE_LOG_INFO("[MultipoolMemoryResource]: Deallocating from upstream.");
        m_Upstream->deallocate(p, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};

}  // namespace Core::MemoryResource