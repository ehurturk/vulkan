#include "platform/core/PlatformEntryPoint.hpp"
#include "platform/core/PlatformContext.hpp"
#include "platform/unix/context.hpp"

std::unique_ptr<Platform::PlatformContext> create_platform_context(int argc, char** argv) {
    return std::make_unique<Platform::UnixPlatformContext>(argc, argv);  // nrvo
}