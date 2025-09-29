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
    int platform_main(const Platform::PlatformContext&);                                       \
    int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,         \
                         INT nCmdShow) {                                                       \
        auto context = create_platform_context(hInstance, hPrevInstance, lpCmdLine, nCmdShow); \
        return platform_main(*context);                                                        \
    }                                                                                          \
    int platform_main(const Platform::PlatformContext& context_name)

#elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MACOS__)
extern std::unique_ptr<Platform::PlatformContext> create_platform_context(int argc, char** argv);

#define ENTRYPOINT(context_name)                            \
    int platform_main(const Platform::PlatformContext&);    \
    int main(int argc, char* argv[]) {                      \
        auto context = create_platform_context(argc, argv); \
        return platform_main(*context);                     \
    }                                                       \
    int platform_main(const Platform::PlatformContext& context_name)

#else
#include <stdexcept>
#define ENTRYPOINT(context_name)                            \
    int main(int argc, char* argv[]) {                      \
        throw std::runtime_error{"platform not supported"}; \
    }                                                       \
    int unused(const Platform::PlatformContext& context_name)
#endif