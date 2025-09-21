#pragma once

#include "defines.hpp"

// A double ended stack allocator for better memory
// allocation, avoids fragmentation as much as possible.
namespace Core {

enum class MemoryTag;  // forward declare to prevent circular imports

class DestackAllocator {
   public:
    enum class HeapDirection { FRAME_TOP, FRAME_BOTTOM };

    struct Marker {
        U32 mark;
        HeapDirection dir;
    };

    explicit DestackAllocator(U32 stackSize);
    ~DestackAllocator();

    // allocates memory with size @size bytes on the heap @heapnr
    // with tag @tag, with (optional) alignment of @alignment.
    void* alloc(U32 size, HeapDirection heapnr, MemoryTag tag, U32 alignment = 16);

    [[nodiscard]] Marker getMarker(HeapDirection dir) const;

    void freeTo(Marker mark);

    void clear();

    [[nodiscard]] U32 getUsedBytes() const { return m_Top + (m_Size - m_Bottom); }
    [[nodiscard]] U32 getAvailableBytes() const { return m_Bottom - m_Top; }

   private:
    U32 m_Size;

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
    U32 m_Top;
    U32 m_Bottom;

    // actual buffer
    U8* m_Buffer;
};
};  // namespace Core
