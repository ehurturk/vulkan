#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <new>
#include <cstddef>
#include "core/assert.hpp"
#include "core/logger.hpp"
#include <type_traits>

namespace Core {

template <typename T, size_t BlockSize = 64, size_t Alignment = alignof(T)> class PoolAllocator {
    SASSERT_MSG(1, "Block size must be greater than 0");
    SASSERT_MSG(Alignment >= alignof(T), "Alignment must be at least alignof(T)");
    SASSERT_MSG((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

  public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U> struct rebind {
        using other = PoolAllocator<U, BlockSize, Alignment>;
    };

  private:
    union Node {
        Node *next;
        alignas(Alignment) std::byte data[sizeof(T)];
        // TODO: std::aligned_storage???
    };

    struct Block {
        std::unique_ptr<Node[]> memory;
        std::size_t size;

        Block(std::size_t n) : size(n) {
            void *raw = ::operator new(sizeof(Node) * n, std::align_val_t{Alignment});
            memory.reset(static_cast<Node *>(raw)); // TODO: maybe use reinterpret_cast for
                                                    // optimization? (decrease assembly output)

            for (std::size_t i = 0; i < n - 1; ++i) {
                memory[i].next = &memory[i + 1];
            }
            memory[n - 1].next = nullptr;
        }

        ~Block() {
            if (memory) {
                ::operator delete(memory.release(), sizeof(Node) * size,
                                  std::align_val_t{Alignment});
            }
        }

        // delete copy ctor, enable default move ctor (unique_ptr's move called)
        Block(Block &&) noexcept = default;
        Block &operator=(Block &&) noexcept = default;
        Block(const Block &) = delete;
        Block &operator=(const Block &) = delete;
    };

    std::atomic<Node *> m_freeList{nullptr};

    std::vector<Block> m_blocks;
    mutable std::mutex m_blockMutex;

    std::atomic<size_t> m_allocCount{0};
    std::atomic<size_t> m_deallocCount{0};
    std::atomic<size_t> m_totalCapacity{0};

    const size_t m_blockSize;
    const bool m_growable;

  public:
    explicit PoolAllocator(size_t initialBlocks = 1, bool growable = true)
        : m_blockSize(BlockSize), m_growable(growable) {

        if (initialBlocks > 0) {
            reserve(initialBlocks * BlockSize);
        }
    }

    ~PoolAllocator() {
#ifdef BUILD_DEBUG
        size_t allocs = m_allocCount.load();
        size_t deallocs = m_deallocCount.load();
        if (allocs != deallocs) {
            ASSERT_MSG(false, "Memory leak detected in PoolAllocator");
        }
#endif
    }

    // delete copy operations
    PoolAllocator(const PoolAllocator &) = delete;
    PoolAllocator &operator=(const PoolAllocator &) = delete;

    PoolAllocator(PoolAllocator &&) noexcept = default;
    PoolAllocator &operator=(PoolAllocator &&) noexcept = default;

    [[nodiscard]] T *allocate() {
        Node *node = popFreeNode();

        if (!node) {
            if (!m_growable) {
                LOG_FATAL("[PoolAllocator]: Bad alloc!");
                ASSERT(false);
                return nullptr;
            }

            grow();
            node = popFreeNode();

            if (!node) {
                LOG_FATAL("[PoolAllocator]: Bad alloc!");
                ASSERT(false);
                return nullptr;
            }
        }

        m_allocCount.fetch_add(1, std::memory_order_relaxed);
        return reinterpret_cast<T *>(node);
    }

    void deallocate(T *ptr, size_t n = 1) noexcept {
        if (!ptr)
            return;

        ASSERT_MSG(n == 1, "Pool allocator only supports single deallocations");
        ASSERT_MSG(owns(ptr), "Pointer not owned by this allocator");

        Node *node = reinterpret_cast<Node *>(ptr);
        pushFreeNode(node);

        m_deallocCount.fetch_add(1, std::memory_order_relaxed);
    }

    template <typename... Args> void construct(T *ptr, Args &&...args) {
        new (ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T *ptr) noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            ptr->~T();
        }
    }

    void reserve(size_t n) {
        std::lock_guard lock(m_blockMutex);

        size_t current = m_totalCapacity.load(std::memory_order_relaxed);
        if (current >= n)
            return;

        size_t needed = n - current;
        size_t blocksToAdd = (needed + m_blockSize - 1) / m_blockSize;

        for (size_t i = 0; i < blocksToAdd; ++i) {
            addBlock();
        }
    }

    [[nodiscard]] size_t capacity() const noexcept {
        return m_totalCapacity.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t allocated() const noexcept {
        return m_allocCount.load(std::memory_order_relaxed) -
               m_deallocCount.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t available() const noexcept { return capacity() - allocated(); }

    [[nodiscard]] bool owns(const T *ptr) const noexcept {
        if (!ptr)
            return false;

        std::lock_guard lock(m_blockMutex);
        const Node *node = reinterpret_cast<const Node *>(ptr);

        for (const auto &block : m_blocks) {
            const Node *begin = block.memory.get();
            const Node *end = begin + block.size;
            if (node >= begin && node < end) {
                return true;
            }
        }
        return false;
    }

    // WARNING: does not call destructors
    void reset() noexcept {
        std::lock_guard lock(m_blockMutex);

        Node *head = nullptr;
        for (auto &block : m_blocks) {
            for (size_t i = 0; i < block.size; ++i) {
                block.memory[i].next = head;
                head = &block.memory[i];
            }
        }

        m_freeList.store(head, std::memory_order_release);
        m_deallocCount.store(m_allocCount.load(std::memory_order_relaxed),
                             std::memory_order_relaxed);
    }

    void clear() {
        std::lock_guard lock(m_blockMutex);
        m_blocks.clear();
        m_freeList.store(nullptr, std::memory_order_release);
        m_allocCount.store(0, std::memory_order_relaxed);
        m_deallocCount.store(0, std::memory_order_relaxed);
        m_totalCapacity.store(0, std::memory_order_relaxed);
    }

    bool operator==(const PoolAllocator &other) const noexcept { return this == &other; }
    bool operator!=(const PoolAllocator &other) const noexcept { return !(*this == other); }

  private:
    Node *popFreeNode() noexcept {
        Node *head = m_freeList.load(std::memory_order_acquire);

        while (head) {
            Node *next = head->next;
            if (m_freeList.compare_exchange_weak(head, next, std::memory_order_release,
                                                 std::memory_order_acquire)) {
                return head;
            }
        }

        return nullptr;
    }

    void pushFreeNode(Node *node) noexcept {
        Node *head = m_freeList.load(std::memory_order_acquire);

        do {
            node->next = head;
        } while (!m_freeList.compare_exchange_weak(head, node, std::memory_order_release,
                                                   std::memory_order_acquire));
    }

    void addBlock() {
        Block &block = m_blocks.emplace_back(m_blockSize);

        Node *blockHead = block.memory.get();
        Node *blockTail = &block.memory[m_blockSize - 1];

        Node *head = m_freeList.load(std::memory_order_acquire);
        do {
            blockTail->next = head;
        } while (!m_freeList.compare_exchange_weak(head, blockHead, std::memory_order_release,
                                                   std::memory_order_acquire));

        m_totalCapacity.fetch_add(m_blockSize, std::memory_order_relaxed);
    }

    void grow() {
        std::lock_guard lock(m_blockMutex);
        addBlock();
    }
};

template <typename T, size_t BlockSize = 64, size_t Alignment = alignof(T)>
std::unique_ptr<PoolAllocator<T, BlockSize, Alignment>> makePoolAllocator(size_t initialBlocks = 1,
                                                                          bool growable = true) {
    return std::make_unique<PoolAllocator<T, BlockSize, Alignment>>(initialBlocks, growable);
}

} // namespace Core
