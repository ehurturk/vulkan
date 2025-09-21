#include "stack_allocator.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "align_utils.hpp"
#include "memory.hpp"
#include <cstdlib>

namespace Core {

StackAllocator::StackAllocator(U32 stackSize)
    : m_Size(stackSize), m_Top(0), m_Buffer(static_cast<U8*>(malloc(stackSize))) {}

StackAllocator::~StackAllocator() {
    clear();
    if (m_Buffer != nullptr)
        free(m_Buffer);
}

void* StackAllocator::alloc(U32 size, MemoryTag tag, U32 alignment) {
    U32 topAligned = MemoryUtil::AlignTo<U32>(m_Top, alignment);
    if (topAligned + size > m_Size) {
        LOG_ERROR("[StackAllocator]:Size is full");
        return nullptr;
    }
    void* result = topAligned + m_Buffer;
    m_Top = topAligned + size;
    return result;
}

StackAllocator::Marker StackAllocator::getMarker() const {
    return m_Top;
}

void StackAllocator::freeTo(StackAllocator::Marker mark) {
    ASSERT_MSG(mark <= m_Top, "[StackAllocator]:Can't free to future position");
    m_Top = mark;
}

void StackAllocator::clear() {
    m_Top = 0;
}

};  // namespace Core
