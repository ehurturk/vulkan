#pragma once

#include <atomic>
#include <memory>
#include <type_traits>
#include <new>
#include <cstddef>

#include "core/assert.hpp"
#include "core/logger.hpp"

namespace Core {

template <typename T, size_t Alignment = alignof(T)>
class PoolAllocator {
    SASSERT_MSG(1, "Block size must be greater than 0");
    SASSERT_MSG(Alignment >= alignof(T), "Alignment must be at least alignof(T)");
    SASSERT_MSG((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

   public:
    // STL compatibility
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = PoolAllocator<U, Alignment>;
    };

    explicit PoolAllocator(U64 noOfElements) : m_NoElements(noOfElements) {
        size_t bytes = m_NoElements * sizeof(Node);
        bytes = ((bytes + Alignment - 1) / Alignment) * Alignment;
        void* raw = std::aligned_alloc(Alignment, bytes);
        if (!raw) {
            LOG_FATAL("[PoolAllocator]: Bad allocation!");
            ASSERT(false);
            return;
        }
        m_Nodes = static_cast<Node*>(raw);

        for (uint i = 0; i < m_NoElements - 1; ++i) {
            // initialize the list
            m_Nodes[i].next = &m_Nodes[i + 1];
        }

        m_Nodes[m_NoElements - 1].next = nullptr;
        m_freeList.store(&m_Nodes[0]);
    }

    ~PoolAllocator() { std::free(m_Nodes); }

    // delete copy operations
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    // delete move constructor for now
    // TODO: look into allowing move since there is m_Nodes as ptr,
    // but afaik atomics don't have move ctor defined.
    PoolAllocator(PoolAllocator&&) noexcept = delete;
    PoolAllocator& operator=(PoolAllocator&&) noexcept = delete;

    [[nodiscard]] T* allocate() {
        Node* node = popFreeNode();

        if (!node) {
            LOG_FATAL("[PoolAllocator]: Bad alloc!");

            ASSERT(false);
            return nullptr;
        }

        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* ptr) noexcept {
        if (!ptr)
            return;

        ASSERT_MSG(owns(ptr), "[PoolAllocator]:Pointer not owned by this allocator");

        Node* node = reinterpret_cast<Node*>(ptr);
        pushFreeNode(node);
    }

    template <typename... Args>
    void construct(T* ptr, Args&&... args) {
        new (ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T* ptr) noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            ptr->~T();
        }
    }

    [[nodiscard]] bool owns(const T* ptr) const noexcept {
        if (!ptr)
            return false;

        const Node* node = reinterpret_cast<const Node*>(ptr);
        return node >= m_Nodes && node < m_Nodes + m_NoElements;
    }

    // WARNING: does not call destructors
    // see destroy() to call destructors
    void reset() noexcept {
        for (size_t i = 0; i < m_NoElements - 1; ++i) {
            m_Nodes[i].next = &m_Nodes[i + 1];
        }

        m_Nodes[m_NoElements - 1].next = nullptr;

        m_freeList.store(&m_Nodes[0], std::memory_order_release);
    }

    void clear() { m_freeList.store(nullptr, std::memory_order_release); }

    bool operator==(const PoolAllocator& other) const noexcept { return this == &other; }
    bool operator!=(const PoolAllocator& other) const noexcept { return !(this == &other); }

   private:
    union Node {
        Node* next;
        alignas(Alignment) std::byte data[sizeof(T)];  // in place of std::aligned_storage
    };

    U32 m_NoElements;
    Node* m_Nodes;
    std::atomic<Node*> m_freeList{nullptr};

    Node* popFreeNode() noexcept {
        Node* head = m_freeList.load(std::memory_order_acquire);

        while (head) {
            Node* next = head->next;
            if (m_freeList.compare_exchange_weak(head, next, std::memory_order_release,
                                                 std::memory_order_acquire)) {
                return head;
            }
        }

        return nullptr;
    }

    void pushFreeNode(Node* node) noexcept {
        Node* head = m_freeList.load(std::memory_order_acquire);

        do {
            node->next = head;
        } while (!m_freeList.compare_exchange_weak(head, node, std::memory_order_release,
                                                   std::memory_order_acquire));
    }
};

template <typename T, size_t Alignment = alignof(T)>
std::unique_ptr<PoolAllocator<T, Alignment>> makePoolAllocator(U32 noelements) {
    return std::make_unique<PoolAllocator<T, Alignment>>(noelements);
}

}  // namespace Core
