#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "defines.h"

typedef struct platform_state {
    void* internal_state;
} platform_state_t;

/* will use window.h functionality */
b8 platform_startup(platform_state_t* state, const char* name, i32 width,
                    i32 height);

void platform_shutdown(platform_state_t* state);
b8 platform_pump_messages(platform_state_t* state);

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

#endif
