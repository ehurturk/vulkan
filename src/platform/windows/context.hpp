#pragma once

#include <Windows.h>
#include "platform/core/PlatformContext.hpp"

namespace Platform {
class WindowsPlatformContext final : public PlatformContext {
   public:
    WindowsPlatformContext(HINSTANCE hInstance,
                           HINSTANCE hPrevInstance,
                           PSTR lpCmdLine,
                           INT nCmdShow);
    ~WindowsPlatformContext() override = default;
};
};  // namespace Platform