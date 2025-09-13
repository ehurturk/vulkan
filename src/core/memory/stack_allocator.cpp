#include "stack_allocator.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "align_utils.hpp"
#include "memory.hpp"
#include <cstdlib>

namespace Core {

StackAllocator::StackAllocator(u32 stackSize)
    : m_Size(stackSize), m_Top(0), m_Buffer(static_cast<u8 *>(malloc(stackSize))) {}

StackAllocator::~StackAllocator() {
    clear();
    if (m_Buffer != nullptr)
        free(m_Buffer);
}

void *StackAllocator::alloc(u32 size, MemoryTag tag, u32 alignment) {
    LOG_INFO("[StackAllocator]:Allocating {} bytes for tag: {}", size, memoryTagToString(tag));
    u32 topAligned = Align::alignAddress(m_Top, alignment);
    if (topAligned + size > m_Size) {
        LOG_ERROR("[StackAllocator]:Size is full");
        return nullptr;
    }
    void *result = topAligned + m_Buffer;
    m_Top = topAligned + size;
    return result;
}

StackAllocator::Marker StackAllocator::getMarker() const { return m_Top; }

void StackAllocator::freeTo(StackAllocator::Marker mark) {
    ASSERT_MSG(mark <= m_Top, "[StackAllocator]:Can't free to future position");
    m_Top = mark;
}

void StackAllocator::clear() { m_Top = 0; }

}; // namespace Core
