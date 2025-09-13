#pragma once

#include "core/assert.hpp"

#include <cstdlib>
#include <memory>

namespace Core {
namespace Align {
inline uintptr_t alignAddress(uintptr_t addr, size_t align) {
    const size_t mask = align - 1;
    ASSERT((align & mask) == 0);
    return (addr + mask) & ~mask;
}

template <typename T> inline T *alignPtr(T *ptr, size_t align) {
    const uintptr_t addr = std::reinterpret_pointer_cast<uintptr_t>(ptr);
    const uintptr_t addrAligned = alignAddress(addr, align);
    return std::reinterpret_pointer_cast<T *>(addrAligned);
}
}; // namespace Align
}; // namespace Core
