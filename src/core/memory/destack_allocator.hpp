#pragma once

#include "defines.hpp"
#include "align_utils.hpp"
#include "core/logger.hpp"

namespace Core::Allocator {

enum class MemoryTag;

class DestackAllocator {
   public:
    enum class HeapDirection { FRAME_TOP, FRAME_BOTTOM };

    struct Marker {
        U32 mark;
        HeapDirection dir;
    };

    explicit DestackAllocator(U32 stackSize)
        : m_Size(stackSize),
          m_Top(0),
          m_Bottom(m_Size),
          m_Buffer(static_cast<U8*>(malloc(m_Size))) {}
    ~DestackAllocator() {
        clear();
        if (m_Buffer != nullptr) {
            free(m_Buffer);
        }
    }

    // allocates memory with size @size bytes on the heap @heapnr
    // with tag @tag, with (optional) alignment of @alignment.
    void* alloc(U32 size, HeapDirection heapnr, U32 alignment = 16) {
        CORE_LOG_INFO("[StackAllocator]:Allocating {} bytes.", size);
        const U32 topAligned = MemoryUtil::AlignTo<U32>(m_Top, alignment);
        const U32 bottomAligned = MemoryUtil::AlignTo<U32>(m_Bottom, alignment);

        if (bottomAligned - topAligned < size) {
            CORE_LOG_FATAL("[DestackAllocator]: Out of pool memory!");
            throw std::bad_alloc();
        }

        U8* result;

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
                CORE_LOG_ERROR("[DestackAllocator]: Invalid Heap Direction: {}",
                               static_cast<int>(heapnr));
                return nullptr;
        }
        return static_cast<void*>(result);
    }

    [[nodiscard]] Marker getMarker(HeapDirection dir) const {
        Marker m{};
        m.dir = dir;
        // Note:
        // if FRAME_TOP is specified, we return the bottom (which grows downwards - essentially acts
        // like the top frame). if FRAME_BOTTOM is specified, we reutrn the top (which grows
        // upwards,
        // - essentially acts like the bottom frame).
        switch (dir) {
            case HeapDirection::FRAME_TOP:
                m.mark = m_Bottom;
                break;
            case HeapDirection::FRAME_BOTTOM:
                m.mark = m_Top;
                break;
            default:
                CORE_LOG_ERROR("[DestackAllocator]: Invalid Heap Direction: {}",
                               static_cast<int>(dir));
                return {};
        }

        return m;
    }

    void freeTo(Marker mark) {
        switch (mark.dir) {
            case HeapDirection::FRAME_TOP:
                m_Bottom = mark.mark;
                break;
            case HeapDirection::FRAME_BOTTOM:
                m_Top = mark.mark;
                break;
        }
    }

    void clear() {
        m_Top = 0;
        m_Bottom = m_Size;
    }

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
};  // namespace Core::Allocator
