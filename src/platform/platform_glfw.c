#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "core/logger.h"
#include "platform.h"
#include "window.h"

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

/*
 * A note on this design:
 * I am currently using GLFW to handle platform-agnostic window handling,
 * while in the future I may switch to platform-dependent functioanlities,
 * such as Linux X11 window creation or Windows Win32 API. In order to have such
 * flexibility while in the meantime allow myself to accelerate this project,
 * I have created platform_glfw to "simulate" GLFW as a platform, while it merely
 * serves the purpose of creating and operating window functionalities. Therefore,
 * this file and window.h/c is ONLY related to GLFW.
 */

typedef struct internal_state {
    window_t* window;
    b8 initialized;
} internal_state_t;

b8 platform_startup(platform_state_t* state, const char* name, i32 width, i32 height) {
    state->internal_state = malloc(sizeof(internal_state_t));
    internal_state_t* istate = (internal_state_t*)state->internal_state;
    istate->initialized = FALSE;

    window_t* win = window_init(width, height, name, FALSE, FALSE);
    if (win == NULL) {
        LOG_FATAL("Failed to initialize window!");
        return FALSE;
    }

    if (!window_create(win)) {
        LOG_FATAL("Failed to create window!");
        return FALSE;
    }

    istate->initialized = TRUE;
    istate->window = win;
    return TRUE;
}

b8 platform_should_run(platform_state_t* state) {
    return window_should_close(((internal_state_t*)state->internal_state)->window);
}

void platform_shutdown(platform_state_t* state) {
    internal_state_t* istate = (internal_state_t*)state->internal_state;
    if (istate->initialized) {
        window_shutdown(istate->window);
    }
    platform_free(istate->window, FALSE);
    istate->window = NULL;
    platform_free(state->internal_state, FALSE);
}

b8 platform_dispatch_messages(platform_state_t* state) {
    window_poll_events();
    return TRUE;
}

void platform_console_write(const char* msg, const char* color) {
    printf("%s%s%s\n", color, msg, FORMAT_RESET);
}

f64 platform_get_abs_time() { return glfwGetTime(); }

void platform_sleep(u64 ms) {
/* TODO: Do it the native OS-specific way (currently doing Linux specific) */
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, NULL);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

/* TODO: Implement a custom (double) stack allocator that prevents fragmentation */
/* TODO: Manually change every call to platform_xxx once custom memory allocators are implemented */
void* platform_allocate(u64 size, b8 aligned) { return malloc(size); }
void platform_free(void* block, b8 aligned) { free(block); }
void* platform_zero_memory(void* block, u64 size) { return memset(block, 0, size); }
void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}
void* platform_set_memory(void* dest, i32 value, u64 size) { return memset(dest, value, size); }
