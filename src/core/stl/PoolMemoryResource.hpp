#pragma once

#include <memory>
#include <memory_resource>

#include "core/memory/pool_allocator.hpp"

namespace Core {
template <typename T>
class PoolMemoryResource : public std::pmr::memory_resource {
   public:
    explicit PoolMemoryResource(PoolAllocator<T>* allocator) : m_Pool(allocator) {};
    explicit PoolMemoryResource(size_t count) : m_Pool(makePoolAllocator<T>(count)) {}

    size_t getAllocCount() const {
#ifdef BUILD_DEBUG
        return m_AllocCount.load();
#else
        return 0;
#endif
    }
    size_t getDeallocCount() const {
#ifdef BUILD_DEBUG
        return m_DeallocCount.load();
#else
        return 0;
#endif
    }

   protected:
    void* do_allocate(size_t bytes, size_t alignment) override {
        if (bytes == sizeof(T)) {
            LOG_INFO("Pool allocating");
            return m_Pool->allocate();
        }
        LOG_INFO("Default allocation");
#ifdef BUILD_DEBUG
        ++m_AllocCount;
#endif
        return std::pmr::get_default_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
        if (bytes == sizeof(T) && m_Pool->owns(static_cast<T*>(ptr))) {
            m_Pool->deallocate(static_cast<T*>(ptr));
        } else {
            std::pmr::get_default_resource()->deallocate(ptr, bytes, alignment);
        }
#ifdef BUILD_DEBUG
        ++m_DeallocCount;
#endif
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

   private:
    std::unique_ptr<PoolAllocator<T>> m_Pool;
#ifdef BUILD_DEBUG
    std::atomic<int> m_AllocCount{0};
    std::atomic<int> m_DeallocCount{0};
#endif
};
}  // namespace Core