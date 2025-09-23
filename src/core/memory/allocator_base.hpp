#pragma once
#include <memory>

namespace Core::Allocator {
class AllocatorBase {
   public:
    [[nodiscard]] virtual void* allocate();
    virtual void deallocate(void* ptr);
};
}  // namespace Core::Allocator