#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "defines.h"

/* Size: 8 bytes */
typedef struct platform_state {
    void* internal_state; /* 8 bytes */
} platform_state_t;

API b8 platform_startup(platform_state_t* state, const char* name, i32 width, i32 height);
API void platform_shutdown(platform_state_t* state);
API b8 platform_dispatch_messages(platform_state_t* state);

void platform_console_write(const char* msg, const char* colour);

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

f64 platform_get_abs_time();
void platform_sleep(u64 ms);

/* FIXME: Remove this */
API b8 platform_should_run(platform_state_t* state);
#endif
