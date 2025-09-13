#include "destack_allocator.hpp"
#include "align_utils.hpp"
#include "memory.hpp"
#include "core/logger.hpp"

namespace Core {

DestackAllocator::DestackAllocator(u32 stackSize)
    : m_Size(stackSize), m_Top(0), m_Bottom(m_Size), m_Buffer(static_cast<u8 *>(malloc(m_Size))) {}

DestackAllocator::~DestackAllocator() {
    clear();
    if (m_Buffer != nullptr) {
        free(m_Buffer);
    }
}

void *DestackAllocator::alloc(u32 size, HeapDirection heapnr, MemoryTag tag, u32 alignment) {
    LOG_INFO("[StackAllocator]:Allocating {} bytes for tag: {}", size, memoryTagToString(tag));
    u32 topAligned = Align::alignAddress(m_Top, alignment);
    u32 bottomAligned = Align::alignAddress(m_Bottom, alignment);

    if (bottomAligned - topAligned <= size) {
        LOG_ERROR("[DetackAllocator]: Size is full");
        return nullptr;
    }

    u8 *result;

    switch (heapnr) {
    case HeapDirection::FRAME_TOP:
        // allocate from upper bottom frame to top frame
        result = m_Buffer + bottomAligned;
        m_Bottom = bottomAligned - size;
        break;
    case HeapDirection::FRAME_BOTTOM:
        result = m_Buffer + topAligned;
        m_Top = topAligned + size;
        break;
    default:
        LOG_ERROR("[DestackAllocator]: Invalid Heap Direction: {}", static_cast<int>(heapnr));
        return nullptr;
    }
    return static_cast<void *>(result);
}

DestackAllocator::Marker DestackAllocator::getMarker(HeapDirection dir) const {
    Marker m;
    m.dir = dir;
    // Note:
    // if FRAME_TOP is specified, we return the bottom (which grows downwards - essentially acts
    // like the top frame). if FRAME_BOTTOM is specified, we reutrn the top (which grows upwards,
    // - essentially acts like the bottom frame).
    switch (dir) {
    case HeapDirection::FRAME_TOP:
        m.mark = m_Bottom;
        break;
    case HeapDirection::FRAME_BOTTOM:
        m.mark = m_Top;
        break;
    default:
        LOG_ERROR("[DestackAllocator]: Invalid Heap Direction: {}", static_cast<int>(dir));
        return {};
        break;
    }

    return m;
}

void DestackAllocator::freeTo(Marker mark) {
    switch (mark.dir) {
    case HeapDirection::FRAME_TOP:
        m_Bottom = mark.mark;
        break;
    case HeapDirection::FRAME_BOTTOM:
        m_Top = mark.mark;
        break;
    }
}

void DestackAllocator::clear() {
    m_Top = 0;
    m_Bottom = 0;
}

}; // namespace Core
