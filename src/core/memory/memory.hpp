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

class MemoryAllocator {
  public:
    MemoryAllocator();
    ~MemoryAllocator();

    API void *allocate(u64 size, MemoryTag tag);
    API void free(void *block, u64 size, MemoryTag tag);
    API void *zero_memory(void *block, u64 size);
    API void *copy_memory(void *dest, const void *source, u64 size);
    API void *set_memory(void *block, i32 value, u64 size);
};

}; // namespace Core
