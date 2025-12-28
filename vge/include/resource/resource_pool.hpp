#pragma once

#include "defines.hpp"
#include "handle.hpp"
#include "core/logger.hpp"

namespace Resource {

template <typename Resource> class ResourcePool {
public:
    Handle allocate(Resource value) {
        Handle handle {};

        // find empty slot

        return handle;
    }

    Resource* get(Handle h) {
        Slot& resource_slot = m_Slots[h.index];

        if (resource_slot.generation != h.generation) {
            CORE_LOG_ERROR(
                "[ResourcePool]: Resource with handle index {} is not found in the resource pool!",
                static_cast<U32>(h.index));
        }

        return &resource_slot.value;
    }

    void free(Handle h) {
        Slot& resource_slot = m_Slots[h.index];
        resource_slot.generation++; // bump the generation
    }

private:
    // disable copy operations
    ResourcePool(const ResourcePool&) = delete;
    ResourcePool& operator=(const ResourcePool&) = delete;

    struct Slot {
        Resource value;
        U32 generation; // bumped when a handle is released
    };

    std::vector<U32> m_FreeList;
    std::vector<Slot> m_Slots;
};

} // namespace Resource