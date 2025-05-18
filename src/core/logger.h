#ifndef LOGGER_H_
#define LOGGER_H_

#include "defines.h"
/* FATAL & ERROR will always log */
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1
typedef enum log_level {
    LOG_LEVEL_FATAL, /* things that do not allow the app to run */
    LOG_LEVEL_ERROR, /* serious error, but app can recover */
    LOG_LEVEL_WARN,  /* warning */
    LOG_LEVEL_INFO,  /* info */
    LOG_LEVEL_DEBUG, /* debug */
    LOG_LEVEL_TRACE  /* verbose debugging */
} log_level_t;

b8 init_log();
void shutdown_log();

API void log_output(log_level_t level, const char* msg, ...);

#ifndef FATAL
#define FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#endif

#ifndef ERROR
#define ERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define WARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define WARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define INFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define TRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
#define TRACE(message, ...)
#endif

#endif