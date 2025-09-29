#pragma once

#include "platform/core/PlatformContext.hpp"
#if defined(__PLATFORM_WINDOWS__)
#include "windows/platform.hpp"
#elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MACOS__)
#include "unix/platform.hpp"
#else
#error "PLATFORM NOT SUPPORTED!"
#endif

namespace Platform {
// class AppPlatform {
// public:
//     AppPlatform(PlatformContext& context);
//     ~AppPlatform() = default;
// private:

// PlatformContext& m_Context;

// #if defined(__PLATFORM_WINDOWS__)
//     WindowsPlatform m_Platform;
// #elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MACOS__)
//     UnixPlatform m_Platform;
// #endif

// };

// inline AppPlatform::AppPlatform(PlatformContext& context): m_Context(context),
// m_Platform(m_Context) {
// }
}  // namespace Platform
