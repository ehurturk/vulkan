#include "renderer/backend/renderer.h"
#include <core/application.h>
#include <core/assert.h>
#include <core/logger.h>
#include <platform/platform.h>
#include <platform/window.h>
#include <types/game_types.h>

// NOTE: This file is a playground that tests the engine library functionality.
int main() {
    LOG_FATAL("Fatal!");
    LOG_ERROR("Error!");
    LOG_WARN("Warning!");
    LOG_INFO("Info!");
    LOG_DEBUG("Debug!");
    LOG_TRACE("Trace!");
    ASSERT(10 > 2);

    game_t game;

    application_create(&(application_config_t){.width = 800, .height = 600, .name = "TestDoga"});

    platform_state_t pstate;

    renderer_t renderer = (renderer_t){NULL, RENDERER_BACKEND_VK};
    init_renderer(&renderer);

    if (platform_startup(&pstate, "Test", 600, 800)) {
        while (!platform_should_run(&pstate)) {
            platform_dispatch_messages(&pstate);
        }
    }

    destroy_renderer(&renderer);
    platform_shutdown(&pstate);
}
