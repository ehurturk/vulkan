#pragma once

#include "defines.hpp"

// A double ended stack allocator for better memory
// allocation, avoids fragmentation as much as possible.
namespace Core {

enum class MemoryTag; // forward declare to prevent circular imports

class DestackAllocator {
  public:
    enum class HeapDirection { FRAME_TOP, FRAME_BOTTOM };

    struct Marker {
        u32 mark;
        HeapDirection dir;
    };

    explicit DestackAllocator(u32 stackSize);
    ~DestackAllocator();

    // allocates memory with size @size bytes on the heap @heapnr
    // with tag @tag, with (optional) alignment of @alignment.
    void *alloc(u32 size, HeapDirection heapnr, MemoryTag tag, u32 alignment = 16);

    Marker getMarker(HeapDirection dir) const;

    void freeTo(Marker mark);

    void clear();

    u32 getUsedBytes() const { return m_Top; }
    u32 getAvailableBytes() const { return m_Size - m_Top; }

  private:
    u32 m_Size;

    // top grows, bottom decreases
    // [  |        |     ]
    //    ^ top    ^ bottom
    // after an allocation in top:
    // [     |     |     ]
    //       ^     ^
    //       t     b
    // after an allocation in bottom:
    // [     |   |       ]
    //       ^   ^
    //       t   b
    u32 m_Top;
    u32 m_Bottom;

    // actual buffer
    u8 *m_Buffer;
};
}; // namespace Core
