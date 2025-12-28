#pragma once

#include <memory_resource>

#include "core/memory/pool_allocator.hpp"

namespace Core::MemoryResource {
using namespace Core::Allocator;

class FixedPoolResource final : public std::pmr::memory_resource {
   public:
    explicit FixedPoolResource(
        FixedPoolAllocator& pool,
        std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        : m_Pool(&pool), m_Upstream(upstream) {}

    FixedPoolAllocator& pool() const noexcept { return *m_Pool; }

   protected:
    void* do_allocate(const std::size_t bytes, const std::size_t align) override {
        if (bytes <= m_Pool->block_size() && align <= m_Pool->block_align()) {
            CORE_LOG_INFO("[FixedPoolResource]: Allocating from pool.");
            return m_Pool->allocate_block();
        }
        CORE_LOG_INFO("[FixedPoolResource]: Allocating from upstream.");
        return m_Upstream->allocate(bytes, align);
    }

    void do_deallocate(void* p, const std::size_t bytes, const std::size_t align) override {
        if (p && m_Pool->owns(p) && bytes <= m_Pool->block_size() &&
            align <= m_Pool->block_align()) {
            CORE_LOG_INFO("[FixedPoolResource]: Deallocating from pool.");
            m_Pool->deallocate_block(p);
        } else {
            CORE_LOG_INFO("[FixedPoolResource]: Deallocating from upstream.");
            m_Upstream->deallocate(p, bytes, align);
        }
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

   private:
    FixedPoolAllocator* m_Pool;
    std::pmr::memory_resource* m_Upstream;
};

}  // namespace Core::MemoryResource