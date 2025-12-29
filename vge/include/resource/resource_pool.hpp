#pragma once

#include "defines.hpp"
#include "handle.hpp"
#include "core/logger.hpp"
#include "core/assert.hpp"

namespace Resource {

// ResourcePool is an *owning* container of
// pool of Resource instances.
template <typename Resource> class ResourcePool {
public:
    ResourcePool() = default;

    // sink:
    //  - with lvalue parameters, resulting operations are 1 copy, 1 move, 1 destruction
    //  - with rvalue parameters (i.e. allocate(std::move(value))), resulting operations are 2 moves
    //    and a destructor
    Handle allocate(Resource value) { return emplace(std::move(value)); }

    // lvalue bind: 1 copy into the slot
    Handle insert(const Resource& value) { return emplace(value); }
    // rvalue bind: 1 move into the slot
    Handle insert(Resource&& value) { return emplace(std::move(value)); }

    // emplace: 0 copy/moves, 1 constructing in-place
    template <class... Args> Handle emplace(Args&&... args) {
        U32 idx = 0;
        Slot* slot = nullptr;

        if (!m_FreeList.empty()) {
            // Use the free list if a free slot is available
            idx = m_FreeList.front();
            m_FreeList.pop();

            slot = &m_Slots[idx];

            ASSERT_MSG(!slot->value.has_value(), "[ResourcePool]: Slot should be empty");
        } else {
            // Allocate a new slot if free list is empty
            idx = static_cast<U32>(m_Slots.size());
            m_Slots.emplace_back();
            slot = &m_Slots.back();
        }

        slot->value.emplace(std::forward<Args>(args)...);

        return Handle::make(idx, slot->generation);
    }

    Resource* get(Handle h) {
        if (h.index() >= m_Slots.size())
            return nullptr;

        Slot& resource_slot = m_Slots[h.index()];

        if (!resource_slot.value.has_value() || resource_slot.generation != h.gen()) {
            return nullptr;
        }

        return &resource_slot.value.value();
    }

    void free(Handle h) {
        if (h.index() >= m_Slots.size()) {
            CORE_LOG_WARN("[ResourcePool]: Handle index is larger than slots?");
            return;
        }

        Slot& slot = m_Slots[h.index()];

        if (slot.generation != h.gen()) {
            CORE_LOG_WARN("[ResourcePool]: Handle index is outdated.");
            return;
        }

        slot.value.reset(); // TODO: queue destruction
        slot.generation++; // bump the generation

        // if a generation counter overflow occurs, disable the resource slot
        if (slot.generation == ((1 << Handle::GenBits) - 1)) {
            CORE_LOG_WARN("[ResourcePool]: Slot with index {} is overflown!", h.index());
            return;
        }

        m_FreeList.push(h.index());
    }

    size_t size() const { return m_Slots.size() - m_FreeList.size(); }

private:
    ResourcePool(const ResourcePool&) = delete;
    ResourcePool& operator=(const ResourcePool&) = delete;

    struct Slot {
        std::optional<Resource> value = std::nullopt;
        U16 generation = 1; // generation = 0 is a null handle
    };

    std::queue<U32> m_FreeList;
    std::vector<Slot> m_Slots;
};

} // namespace Resource