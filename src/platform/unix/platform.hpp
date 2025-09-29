#pragma once

#include "../platform.hpp"
#include "platform/core/PlatformContext.hpp"

namespace Platform {
enum class UnixType { MACOS, LINUX };
class UnixPlatform : public Platform {
   public:
    UnixPlatform(const PlatformContext& context, UnixType type);
    ~UnixPlatform();

    void createWindow(const Window::Properties& properties);

   private:
    UnixType m_Type;
};
}  // namespace Platform