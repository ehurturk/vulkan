#pragma once

#include "../core/PlatformContext.hpp"

namespace Platform {

class UnixPlatformContext final : public PlatformContext {
   public:
    UnixPlatformContext(int argc, char** argv);
    ~UnixPlatformContext() override = default;
};
}  // namespace Platform