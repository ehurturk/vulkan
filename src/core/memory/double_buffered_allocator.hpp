#pragma once
#include "defines.hpp"
#include "stack_allocator.hpp"

namespace Core::Allocator {

class DoubleBufferedAllocator {
   public:
    explicit DoubleBufferedAllocator(U32 size_)
        : m_CurStack(0), m_Stack({StackAllocator(size_), StackAllocator(size_)}) {}
    void swap_buffers() { m_CurStack = !m_CurStack; }

    void clear_current_buffer() { m_Stack[m_CurStack].clear(); }

    void* alloc(size_t bytes) { return m_Stack[m_CurStack].allocate(bytes); }

   private:
    U8 m_CurStack;
    std::array<StackAllocator, 2> m_Stack;
};
}  // namespace Core::Allocator
