#pragma once

#include "defines.hpp"

namespace Core {

enum class MemoryTag;

class StackAllocator {
  public:
    using Marker = u32;

    explicit StackAllocator(u32 stackSize);
    ~StackAllocator();

    void *alloc(u32 size, MemoryTag tag, u32 alignment = 16);

    Marker getMarker() const;

    void freeTo(Marker mark);

    void clear();

    u32 getUsedBytes() const { return m_Top; }
    u32 getAvailableBytes() const { return m_Size - m_Top; }

  private:
    u32 m_Size;
    u32 m_Top;

    // actual buffer
    u8 *m_Buffer;
};

}; // namespace Core
