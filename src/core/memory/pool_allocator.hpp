#pragma once

#include <memory>
#include <type_traits>
#include <new>
#include <cstddef>

#include "align_utils.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"

namespace Core::Allocator {

template <typename T>
class PoolAllocator {
   public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = PoolAllocator<U>;
    };

    explicit PoolAllocator(const std::size_t blocks) : m_Capacity(blocks) {
        ASSERT_MSG(blocks > 0, "[PoolAllocator]: Pool capacity must be greater than zero.");

        std::size_t total_bytes = m_Capacity * BlockSize;
        total_bytes = Core::MemoryUtil::RoundToAlignment(total_bytes, Alignment);
        m_Buffer = static_cast<std::byte*>(
            ::operator new(total_bytes, static_cast<std::align_val_t>(Alignment)));

        m_FreeHead = nullptr;
        for (std::size_t i = 0; i < m_Capacity; ++i) {
            auto* current_block = reinterpret_cast<Node*>(m_Buffer + i * BlockSize);
            current_block->next = m_FreeHead;
            m_FreeHead = current_block;
        }
    }

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;

    ~PoolAllocator() {
        ::operator delete(m_Buffer, static_cast<std::align_val_t>(Alignment));
        m_Buffer = nullptr;
        m_FreeHead = nullptr;
    }

    [[nodiscard]] T* allocate() {
        if (!m_FreeHead) {
            throw std::bad_alloc();
        }

        Node* n = m_FreeHead;
        m_FreeHead = n->next;
        ++m_InUse;

        return reinterpret_cast<T*>(n);
    }
    void deallocate(T* p) noexcept {
        if (!p) {
            return;
        }

        auto* n = reinterpret_cast<Node*>(p);
        n->next = m_FreeHead;
        m_FreeHead = n;
        --m_InUse;
    }

    template <typename... Args>
    [[nodiscard]] T* create(Args&&... args) {
        T* p = allocate();
        try {
            new (p) T(std::forward<Args>(args)...);
        } catch (...) {
            deallocate(p);
            throw;
        }
        return p;
    }

    template <typename... Args>
    void construct(T* ptr, Args&&... args) noexcept {
        new (ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T* p) noexcept {
        if (!p) {
            return;
        }
        p->~T();
        deallocate(p);
    }

    std::size_t capacity() const noexcept { return m_Capacity; }
    std::size_t in_use() const noexcept { return m_InUse; }
    std::size_t free_blocks() const noexcept { return m_Capacity - m_InUse; }

   private:
    struct Node {
        Node* next;
    };

    static constexpr size_t BlockSize = sizeof(T) > sizeof(Node) ? sizeof(T) : sizeof(Node);
    static constexpr size_t Alignment = alignof(T);

    SASSERT_MSG(Core::MemoryUtil::IsPowerOfTwo(Alignment),
                "[PoolAllocator]: Alignment must be power of 2");

    std::size_t m_Capacity;
    std::byte* m_Buffer{};
    Node* m_FreeHead{};
    std::size_t m_InUse{0};
};

class FixedPoolAllocator {
   public:
    FixedPoolAllocator(const std::size_t block_size,
                       const std::size_t block_align,
                       const std::size_t blocks)
        : m_BlockSize(round_up(block_size, block_align)),
          m_BlockAlign(block_align),
          m_Capacity(blocks) {
        ASSERT_MSG(m_BlockAlign && (m_BlockAlign & (m_BlockAlign - 1)) == 0,
                   "[FixedPoolAllocator]: Alignment must be power of two");
        const std::size_t bytes = round_up(m_BlockSize * m_Capacity, m_BlockAlign);

        m_Base = static_cast<std::byte*>(
            ::operator new(bytes, static_cast<std::align_val_t>(m_BlockAlign)));
        m_End = m_Base + bytes;

        m_Free = nullptr;
        for (std::size_t i = 0; i < m_Capacity; ++i) {
            auto* n = reinterpret_cast<Node*>(m_Base + i * m_BlockSize);
            n->next = m_Free;
            m_Free = n;
        }
    }

    FixedPoolAllocator(const FixedPoolAllocator&) = delete;
    FixedPoolAllocator& operator=(const FixedPoolAllocator&) = delete;

    ~FixedPoolAllocator() {
        ::operator delete(m_Base, static_cast<std::align_val_t>(m_BlockAlign));
        m_Base = m_End = nullptr;
        m_Free = nullptr;
    }

    [[nodiscard]] void* allocate_block() {
        if (!m_Free) {
            LOG_FATAL("[FixedPoolAllocator]: Out of pool memory!");
            throw std::bad_alloc();
        }

        return allocate();
    }

    [[nodiscard]] void* try_allocate_block() noexcept {
        if (!m_Free) {
            LOG_FATAL("[FixedPoolAllocator]: Out of pool memory!");
            return nullptr;
        }

        return allocate();
    }

    void deallocate_block(void* p) noexcept {
        if (!p)
            return;
        auto* n = static_cast<Node*>(p);
        n->next = m_Free;
        m_Free = n;
        --m_InUse;
    }

    bool owns(const void* p) const noexcept {
        auto* b = static_cast<const std::byte*>(p);
        return b >= m_Base && b < m_End;
    }

    std::size_t block_size() const noexcept { return m_BlockSize; }
    std::size_t block_align() const noexcept { return m_BlockAlign; }
    std::size_t capacity() const noexcept { return m_Capacity; }
    std::size_t in_use() const noexcept { return m_InUse; }
    std::size_t free_blocks() const noexcept { return m_Capacity - m_InUse; }

   private:
    struct Node {
        Node* next;
    };

    [[nodiscard]] void* allocate() noexcept {
        Node* n = m_Free;
        m_Free = n->next;
        ++m_InUse;
        return n;
    }

    static std::size_t round_up(std::size_t n, std::size_t a) { return (n + a - 1) / a * a; }

    std::size_t m_BlockSize;
    std::size_t m_BlockAlign;
    std::size_t m_Capacity;

    std::byte* m_Base{};
    std::byte* m_End{};

    Node* m_Free{};
    std::size_t m_InUse{0};
};

}  // namespace Core::Allocator
