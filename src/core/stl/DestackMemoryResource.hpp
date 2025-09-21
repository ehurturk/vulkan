#pragma once

#include <memory_resource>
#include "core/memory/destack_allocator.hpp"

namespace Core::MemoryResource {
using namespace Allocator;
class DestackBottomMemoryResource final : public std::pmr::memory_resource {
   public:
    explicit DestackBottomMemoryResource(DestackAllocator& allocator) : m_Allocator(allocator) {}

   private:
    DestackAllocator& m_Allocator;

    void* do_allocate(size_t bytes, size_t alignment) override {
        return m_Allocator.alloc(static_cast<U32>(bytes),
                                 DestackAllocator::HeapDirection::FRAME_BOTTOM,
                                 static_cast<U32>(alignment));
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        (void)p;
        (void)bytes;
        (void)alignment;
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        const auto* other_ptr = dynamic_cast<const DestackBottomMemoryResource*>(&other);
        return other_ptr && &other_ptr->m_Allocator == &m_Allocator;
    }
};

class DestackTopMemoryResource final : public std::pmr::memory_resource {
   public:
    explicit DestackTopMemoryResource(DestackAllocator& allocator) : m_Allocator(allocator) {}

   private:
    DestackAllocator& m_Allocator;

    void* do_allocate(size_t bytes, size_t alignment) override {
        return m_Allocator.alloc(static_cast<U32>(bytes),
                                 DestackAllocator::HeapDirection::FRAME_TOP,
                                 static_cast<U32>(alignment));
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        (void)p;
        (void)bytes;
        (void)alignment;
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        const auto* other_ptr = dynamic_cast<const DestackTopMemoryResource*>(&other);
        return other_ptr && &other_ptr->m_Allocator == &m_Allocator;
    }
};

}  // namespace Core::MemoryResource