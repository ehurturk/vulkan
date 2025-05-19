#include "platform.h"

#include <memory.h>
#include <stdlib.h>
/*
b8 platform_startup(platform_state_t* state, const char* name, i32 width,
                    i32 height) {}

void platform_shutdown(platform_state_t* state) {}

b8 platform_pump_messages(platform_state_t* state) {
    // TODO: probably replace with window_poll_events();
}

void* platform_allocate(u64 size, b8 aligned) { return malloc(size); }

void platform_free(void* block, b8 aligned) { free(block); }

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}
*/