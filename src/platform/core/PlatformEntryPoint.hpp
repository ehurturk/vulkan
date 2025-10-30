#pragma once

#include "PlatformContext.hpp"

#if defined(__PLATFORM_MACOS__)
#include <TargetConditionals.h>
#endif

#if defined(__PLATFORM_WINDOWS__)
#include <Windows.h>
extern std::unique_ptr<Platform::PlatformContext> create_platform_context(HINSTANCE hInstance,
                                                                          HINSTANCE hPrevInstance,
                                                                          PSTR lpCmdLine,
                                                                          INT nCmdShow);

#define ENTRYPOINT(context_name)                                                               \
    int platform_main(Platform::WindowsPlatform&);                                             \
    int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,         \
                         INT nCmdShow) {                                                       \
        auto context = create_platform_context(hInstance, hPrevInstance, lpCmdLine, nCmdShow); \
        auto platform = Platform::WindowsPlatform();                                           \
        return platform_main(platform);
}
int platform_main(Platform::WindowsPlatform& platform)

#elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MACOS__)

#if defined(__PLATFORM_LINUX__)
#define UNIX_TYPE Platform::UnixType::LINUX
#elif defined(__PLATFORM_MACOS__)
#define UNIX_TYPE Platform::UnixType::MACOS
#endif

extern std::unique_ptr<Platform::PlatformContext> create_platform_context(int argc, char** argv);

#define ENTRYPOINT(context_name)                                \
    int platform_main(Platform::UnixPlatform&);                 \
    int main(int argc, char* argv[]) {                          \
        auto context = create_platform_context(argc, argv);     \
        Platform::UnixType type = UNIX_TYPE;                    \
        auto platform = Platform::UnixPlatform(*context, type); \
        int result = platform_main(platform);                   \
        return result;                                          \
    }                                                           \
    int platform_main(Platform::UnixPlatform& platform)

#else
#include <stdexcept>
#define ENTRYPOINT(context_name)                            \
    int main(int argc, char* argv[]) {                      \
        throw std::runtime_error{"platform not supported"}; \
    }                                                       \
    int unused(const Platform::PlatformContext& context_name)
#endif
