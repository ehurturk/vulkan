#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

CREATE_CONTEXT(playground_context) {
    for (const auto& arg : playground_context.arguments()) {
        LOG_INFO("Arg: {}", arg);
    }

    return 1;
}