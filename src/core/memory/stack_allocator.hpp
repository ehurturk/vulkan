#pragma once

#include "defines.hpp"

namespace Core {

enum class MemoryTag;

class StackAllocator {
   public:
    using Marker = U32;

    explicit StackAllocator(U32 stackSize);
    ~StackAllocator();

    void* alloc(U32 size, MemoryTag tag, U32 alignment = 16);

    Marker getMarker() const;

    void freeTo(Marker mark);

    void clear();

    U32 getUsedBytes() const { return m_Top; }
    U32 getAvailableBytes() const { return m_Size - m_Top; }

   private:
    U32 m_Size;
    U32 m_Top;

    // actual buffer
    U8* m_Buffer;
};

};  // namespace Core
