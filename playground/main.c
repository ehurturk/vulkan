#include <core/assert.h>
#include <core/logger.h>
#include <platform/platform.h>
#include <platform/window.h>

// NOTE: This file is a playground that tests the engine library functionality.
int main() {
    LOG_FATAL("Fatal!");
    LOG_ERROR("Error!");
    LOG_WARN("Warning!");
    LOG_INFO("Info!");
    LOG_DEBUG("Debug!");
    LOG_TRACE("Trace!");
    ASSERT(10 > 2);

    platform_state_t pstate;
    if (platform_startup(&pstate, "Test", 600, 800)) {
        while (!platform_should_run(&pstate)) {
            platform_dispatch_messages(&pstate);
        }
    }

    platform_shutdown(&pstate);
}
