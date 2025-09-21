#pragma once

#include "core/assert.hpp"

#include <cstdlib>
#include <memory>

namespace Core {
namespace MemoryUtil {

template <typename NonPtrType>
constexpr NonPtrType AlignTo(NonPtrType t, size_t align) {
    size_t bump = static_cast<size_t>(t) + (align - 1);
    size_t trunc = bump & ~(align - 1);
    return static_cast<NonPtrType>(trunc);
}

template <typename PtrType>
constexpr PtrType* AlignTo(PtrType* t, size_t align) {
    return reinterpret_cast<PtrType*>(AlignTo(reinterpret_cast<std::uintptr_t>(t), align));
}

template <typename NonPtrType>
constexpr bool IsAligned(NonPtrType t, size_t align) {
    return (t & (align - 1)) == 0;
}
template <typename PtrType>
constexpr bool IsAligned(PtrType* t, size_t align) {
    return (reinterpret_cast<std::uintptr_t>(t) & (align - 1)) == 0;
}

constexpr bool IsPowerOfTwo(const uintptr_t x) {
    return (x & (x - 1)) == 0;
}

};  // namespace MemoryUtil
};  // namespace Core
