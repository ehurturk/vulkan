#include "logger.h"

// TODO: temporary
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

/* Implement report assert here */
#include "assert.h"
#include "platform/platform.h"

static const char* log_level_colors[] = {COLOR_FATAL, COLOR_ERROR, COLOR_WARN,
                                         COLOR_INFO,  COLOR_DEBUG, COLOR_TRACE};

void assertion_report_failure(const char* expr, const char* msg, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: %s, in file: %s, in line: %d",
               expr, msg, file, line);
}

b8 init_log() {
    // TODO: create a log file
    return TRUE;
}

void shutdown_log() {
    // TODO: cleanup logging/write queued entries
}

void log_output(log_level_t level, const char* msg, ...) {
    const char* levels[6] = {
        "[FATAL]:", "[ERROR]:", "[WARNING]:", "[INFO]:", "[DEBUG]:", "[TRACE]:"};
    b8 is_error = level < 2;

    /* 32k char limit on a single log entry */
    char out_msg[32000];
    memset(out_msg, 0, sizeof(out_msg));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(out_msg, 32000, msg, arg_ptr);
    va_end(arg_ptr);

    char out_msg_p[32000];

    sprintf(out_msg_p, "%s%s", levels[level], out_msg);

    platform_console_write(out_msg_p, log_level_colors[(int)level]);
}
