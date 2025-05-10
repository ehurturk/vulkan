#include "logger.h"

// TODO: temporary
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

b8 init_log() {
    // TODO: create a log file
    return TRUE;
}

void shutdown_log() {
    // TODO: cleanup logging/write queued entries
}

API void log_output(log_level level, const char* msg, ...) {
    const char* levels[6] = {"[FATAL]:", "[ERROR]:", "[WARNING]:",
                             "[INFO]:",  "[DEBUG]:", "[TRACE]:"};
    b8 is_error = level < 2;

    /* 32k char limit on a single log entry */
    char out_msg[32000];
    memset(out_msg, 0, sizeof(out_msg));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(out_msg, 32000, msg, arg_ptr);
    va_end(arg_ptr);

    char out_msg_p[32000];

    sprintf(out_msg_p, "%s%s\n", levels[level], out_msg);

    // TODO: Platform specific output
    printf("%s", out_msg_p);
}
