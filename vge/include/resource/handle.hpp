#pragma once

#include "defines.hpp"

namespace Resource {

// A 32-bit handle will contain 8 bits for generation counters,
// meaning 2^32 resources could be created, with at most 2^(32-8)
// = 2^24 alive at the same time.
struct Handle {
    using HandleType = U32;

    HandleType value = 0;

    static constexpr int IndexBits = 16;
    static constexpr int GenBits = 16;
    static constexpr int IndexMask = (1u << IndexBits) - 1u;
    static constexpr int GenMask = (1u << GenBits) - 1u;

    static constexpr Handle null() { return Handle { 0 }; }

    static constexpr Handle make(U32 index, U32 gen) {
        return Handle { (static_cast<HandleType>(index) & IndexMask)
            | ((static_cast<HandleType>(gen) & GenMask) << IndexBits) };
    }

    constexpr U32 index() const { return value & IndexMask; }
    constexpr U32 gen() const { return (value >> IndexBits) & GenMask; }

    constexpr explicit operator bool() const { return value != 0; }

    constexpr bool operator==(Handle other) const noexcept { return value == other.value; }
    constexpr bool operator!=(Handle other) const noexcept { return value != other.value; }
};

template <typename T> class ResourceHandle {
    constexpr ResourceHandle() noexcept
        : m_Handle {} { }
    explicit constexpr ResourceHandle(Handle handle) noexcept
        : m_Handle(handle) { }

    constexpr Handle handle() const noexcept { return m_Handle; }

    constexpr bool operator==(ResourceHandle other) const noexcept {
        return m_Handle == other.m_Handle;
    }

    constexpr bool operator!=(ResourceHandle other) const noexcept {
        return m_Handle != other.m_Handle;
    }

private:
    Handle m_Handle;
};

struct TextureTag { };
using TextureHandle = ResourceHandle<TextureTag>;

struct MaterialTag { };
using MaterialHandle = ResourceHandle<MaterialTag>;

struct MeshTag { };
using MeshHandle = ResourceHandle<MeshTag>;

struct ShaderTag { };
using ShaderHandle = ResourceHandle<ShaderTag>;

static_assert(Handle::IndexBits + Handle::GenBits == sizeof(Handle::HandleType) * 8);

} // namespace Resource

// hash support
namespace std {
template <> struct hash<Resource::Handle> {
    size_t operator()(Resource::Handle id) const noexcept {
        return std::hash<Resource::Handle::HandleType> {}(
            reinterpret_cast<const Resource::Handle::HandleType&>(id));
    }
};

template <typename T> struct hash<Resource::ResourceHandle<T>> {
    size_t operator()(Resource::ResourceHandle<T> handle) const noexcept {
        return std::hash<Resource::Handle> {}(handle.id());
    }
};
}