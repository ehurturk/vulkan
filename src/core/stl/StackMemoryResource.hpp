#pragma once

#include <memory_resource>
#include "core/memory/stack_allocator.hpp"

namespace Core::MemoryResource {
using namespace Allocator;
class StackMemoryResource : public std::pmr::memory_resource {
   public:
    explicit StackMemoryResource(StackAllocator& allocator) : m_Allocator(allocator) {}

   private:
    StackAllocator& m_Allocator;

    void* do_allocate(size_t bytes, size_t alignment) override {
        return m_Allocator.allocate(static_cast<U32>(bytes), static_cast<U32>(alignment));
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        // nop, memory is freed by calling freeTo()/clear() on the allocator
        (void)p;
        (void)bytes;
        (void)alignment;
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        const auto* other_ptr = dynamic_cast<const StackMemoryResource*>(&other);
        return other_ptr && &other_ptr->m_Allocator == &m_Allocator;
    }
};

}  // namespace Core::MemoryResource