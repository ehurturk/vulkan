#pragma once

#include "defines.hpp"

namespace Core {

enum class MemoryTag {
    MEMORY_TAG_UNKNOWN, // temporary
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
};

constexpr std::string_view memoryTagToString(MemoryTag tag) {
    switch (tag) {
    case MemoryTag::MEMORY_TAG_UNKNOWN:
        return "MEMORY_TAG_UNKNOWN";
    case MemoryTag::MEMORY_TAG_ARRAY:
        return "MEMORY_TAG_ARRAY";
    case MemoryTag::MEMORY_TAG_DARRAY:
        return "MEMORY_TAG_DARRAY";
    case MemoryTag::MEMORY_TAG_DICT:
        return "MEMORY_TAG_DICT";
    case MemoryTag::MEMORY_TAG_RING_QUEUE:
        return "MEMORY_TAG_RING_QUEUE";
    case MemoryTag::MEMORY_TAG_BST:
        return "MEMORY_TAG_BST";
    case MemoryTag::MEMORY_TAG_STRING:
        return "MEMORY_TAG_STRING";
    case MemoryTag::MEMORY_TAG_APPLICATION:
        return "MEMORY_TAG_APPLICATION";
    case MemoryTag::MEMORY_TAG_JOB:
        return "MEMORY_TAG_JOB";
    case MemoryTag::MEMORY_TAG_TEXTURE:
        return "MEMORY_TAG_TEXTURE";
    case MemoryTag::MEMORY_TAG_MATERIAL_INSTANCE:
        return "MEMORY_TAG_MATERIAL_INSTANCE";
    case MemoryTag::MEMORY_TAG_RENDERER:
        return "MEMORY_TAG_RENDERER";
    case MemoryTag::MEMORY_TAG_GAME:
        return "MEMORY_TAG_GAME";
    case MemoryTag::MEMORY_TAG_TRANSFORM:
        return "MEMORY_TAG_TRANSFORM";
    case MemoryTag::MEMORY_TAG_ENTITY:
        return "MEMORY_TAG_ENTITY";
    case MemoryTag::MEMORY_TAG_ENTITY_NODE:
        return "MEMORY_TAG_ENTITY_NODE";
    case MemoryTag::MEMORY_TAG_SCENE:
        return "MEMORY_TAG_SCENE";
    case MemoryTag::MEMORY_TAG_MAX_TAGS:
        return "INVALID_TAG";
    default:
        return "UNKNOWN_TAG";
    }
}

class MemoryAllocator {
  public:
    MemoryAllocator();
    ~MemoryAllocator();

    API void *allocate(U64 size, MemoryTag tag);
    API void free(void *block, U64 size, MemoryTag tag);
    API void *zero_memory(void *block, U64 size);
    API void *copy_memory(void *dest, const void *source, U64 size);
    API void *set_memory(void *block, I32 value, U64 size);
};

}; // namespace Core
