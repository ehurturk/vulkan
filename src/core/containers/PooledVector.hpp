#pragma once

#include "core/memory/pool_allocator.hpp"

namespace Core::Containers {
template <typename T>
class PooledVector {
   public:
    explicit PooledVector(size_t maxSize) : m_Pool(maxSize) { m_Elements.reserve(maxSize); }

    ~PooledVector() { clear(); }

    T* emplace_back(auto&&... args) {
        if (m_Elements.size() >= m_Elements.capacity()) {
            return nullptr;
        }
        T* elem = m_Pool.allocate();
        if (!elem) {
            return nullptr;
        }
        m_Pool.construct(elem, std::forward<decltype(args)>(args)...);
        m_Elements.push_back(elem);
        return elem;
    }

    void push_back(const T& elem) {
        T* elem_alloc = m_Pool.allocate();
        *elem_alloc = elem;  // TODO: copy ctor?
        m_Elements.push_back(elem_alloc);
    }

    T* pop_back() {
        if (m_Elements.empty()) {
            return nullptr;
        }
        T* elem = m_Elements.back();
        m_Pool.destroy(elem);
        m_Pool.deallocate(elem);
        m_Elements.pop_back();
        return elem;
    }

    // o(n)
    void remove(T* obj) {
        auto it = std::find(m_Elements.begin(), m_Elements.end(), obj);
        if (it != m_Elements.end()) {
            // O(1) removal
            std::swap(*it, m_Elements.back());
            m_Elements.pop_back();

            m_Pool.destroy(obj);
            m_Pool.deallocate(obj);
        }
    }

    T& operator[](size_t i) { return *m_Elements[i]; }
    size_t size() const { return m_Elements.size(); }
    size_t capacity() const { return m_Elements.capacity(); }
    bool empty() const { return m_Elements.empty(); }

    void clear() {
        for (T* obj : m_Elements) {
            m_Pool.destroy(obj);
            m_Pool.deallocate(obj);
        }
        m_Elements.clear();
    }

    // iterator support
    auto begin() { return m_Elements.begin(); }
    auto end() { return m_Elements.end(); }

   private:
    PoolAllocator<T> m_Pool;
    std::vector<T*> m_Elements;
};
}  // namespace Core::Containers