#pragma once

#include "core/memory/pool_allocator.hpp"

namespace Core::Allocator {
template <class U>
struct PoolAdapter {
    using value_type = U;
    using pointer = U*;
    using const_pointer = const U*;
    using reference = U&;
    using const_reference = const U&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    PoolAllocator<U>* pool = nullptr;

    PoolAdapter() = default;
    explicit PoolAdapter(PoolAllocator<U>& p) : pool(&p) {}

    template <class V>
    PoolAdapter(const PoolAdapter<V>& o) noexcept : pool(o.pool) {}

    U* allocate(std::size_t n) {
        LOG_INFO("Allocating {} bytes", n);
        if (n != 1)
            throw std::bad_alloc();
        void* p = pool->allocate(n);
        if (!p)
            throw std::bad_alloc();
        // alignment of pool must be >= alignof(U)
        return static_cast<U*>(p);
    }
    void deallocate(U* p, std::size_t) noexcept { pool->deallocate(p); }

    template <class V>
    struct rebind {
        using other = PoolAdapter<V>;
    };

    bool operator==(const PoolAdapter& rhs) const noexcept { return pool == rhs.pool; }
    bool operator!=(const PoolAdapter& rhs) const noexcept { return !(*this == rhs); }
};
}  // namespace Core::Allocator