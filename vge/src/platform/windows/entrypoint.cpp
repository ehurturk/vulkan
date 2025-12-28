#include "platform/core/PlatformContext.hpp"
#include "platform/core/PlatformEntryPoint.hpp"
#include "platform/unix/context.hpp"

extern std::unique_ptr<Platform::PlatformContext> create_platform_context(HINSTANCE hInstance,
                                                                          HINSTANCE hPrevInstance,
                                                                          PSTR lpCmdLine,
                                                                          INT nCmdShow) {
    return std::make_unique<Platform::WindowsPlatformContext>(hInstance, hPrevInstance, lpCmdLine,
                                                              nCmdShow);
}
