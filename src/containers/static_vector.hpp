#pragma once

#include <memory>
#include <new>
#include <type_traits>

#include "core/logger.hpp"

template <class Type, std::size_t N> class static_vector {
  public:
    template <typename... Args> void emplace_back(Args &&...args) {
        if (m_Size >= N) {
            // bad alloc
            LOG_FATAL("[StaticVector]: Bad alloc");
            return;
        }

        ::new (&data[m_Size]) Type(std::forward<Args>(args)...);
        ++m_Size;
    }

    const Type &operator[](std::size_t pos) const {
        return *std::launder(reinterpret_cast<const Type *>(&data[pos]));
    }

    ~static_vector() {
        for (std::size_t pos = 0; pos < m_Size; ++pos) {
            std::destroy_at(std::launder(reinterpret_cast<Type *>(&data[pos])));
        }
    }

  private:
    std::size_t m_Size = 0;
    std::aligned_storage<sizeof(Type), alignof(Type)> data[N];
};
