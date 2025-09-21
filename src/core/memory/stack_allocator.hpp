#pragma once

#include "defines.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "align_utils.hpp"
#include <cstdlib>

namespace Core::Allocator {

enum class MemoryTag;

class StackAllocator {
   public:
    using Marker = U32;

    explicit StackAllocator(const U32 stackSize)
        : m_Size(stackSize), m_Top(0), m_Buffer(static_cast<U8*>(malloc(stackSize))) {}
    ~StackAllocator() {
        clear();
        if (m_Buffer != nullptr)
            free(m_Buffer);
    }

    [[nodiscard]] void* alloc(U32 size, U32 alignment = 16) {
        U32 topAligned = MemoryUtil::AlignTo<U32>(m_Top, alignment);
        if (topAligned + size > m_Size) {
            LOG_FATAL("[StackAllocator]: Out of pool memory!");
            throw std::bad_alloc();
        }
        void* result = topAligned + m_Buffer;
        m_Top = topAligned + size;
        return result;
    }

    Marker getMarker() const { return m_Top; }

    void freeTo(Marker mark) {
        ASSERT_MSG(mark <= m_Top, "[StackAllocator]:Can't free to future position");
        m_Top = mark;
    }

    void clear() { m_Top = 0; }

    U32 getUsedBytes() const { return m_Top; }
    U32 getAvailableBytes() const { return m_Size - m_Top; }

   private:
    U32 m_Size;
    U32 m_Top;

    // actual buffer
    U8* m_Buffer;
};

};  // namespace Core::Allocator
