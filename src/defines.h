#ifndef DEFINES_H_
#define DEFINES_H_

/* Use stdint to have guaranteed sizes */
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef int b32; /* 32 bit bool */
typedef char b8; /* 8 bit bool */

#define COLOR_BLACK "\033[0;30m"
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_WHITE "\033[0;37m"

#define COLOR_BOLD_BLACK "\033[1;30m"
#define COLOR_BOLD_RED "\033[1;31m"
#define COLOR_BOLD_GREEN "\033[1;32m"
#define COLOR_BOLD_YELLOW "\033[1;33m"
#define COLOR_BOLD_BLUE "\033[1;34m"
#define COLOR_BOLD_MAGENTA "\033[1;35m"
#define COLOR_BOLD_CYAN "\033[1;36m"
#define COLOR_BOLD_WHITE "\033[1;37m"

#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"

#define FORMAT_RESET "\033[0m"
#define FORMAT_BOLD "\033[1m"
#define FORMAT_UNDERLINE "\033[4m"
#define FORMAT_BLINK "\033[5m"
#define FORMAT_REVERSE "\033[7m"

#define COLOR_FATAL COLOR_BOLD_WHITE BG_RED /* White on red background */
#define COLOR_ERROR COLOR_BOLD_RED          /* Bright red */
#define COLOR_WARN COLOR_BOLD_YELLOW        /* Bright yellow */
#define COLOR_INFO COLOR_BOLD_GREEN         /* Bright green */
#define COLOR_DEBUG COLOR_BOLD_CYAN         /* Bright cyan */
#define COLOR_TRACE COLOR_BOLD_BLUE         /* Bright blue */

#define COLOR_SUCCESS COLOR_BOLD_GREEN     /* Success messages */
#define COLOR_HIGHLIGHT COLOR_BOLD_MAGENTA /* Highlighting important info */
#define COLOR_SYSTEM COLOR_BOLD_WHITE      /* System operations */
#define COLOR_MEMORY COLOR_BOLD_CYAN       /* Memory operations */
#define COLOR_IO COLOR_BOLD_YELLOW         /* I/O operations */
#define COLOR_NETWORK COLOR_BOLD_BLUE      /* Network operations */
#define COLOR_GRAPHICS COLOR_BOLD_MAGENTA  /* Graphics operations */
#define COLOR_AUDIO COLOR_BOLD_GREEN       /* Audio operations */
#define COLOR_PHYSICS COLOR_BOLD_YELLOW    /* Physics operations */
#define COLOR_INPUT COLOR_BOLD_CYAN        /* Input handling */

#define COLOR_CRITICAL BG_RED COLOR_WHITE     /* White on red */
#define COLOR_IMPORTANT BG_YELLOW COLOR_BLACK /* Black on yellow */
#define COLOR_SUCCESS_BG BG_GREEN COLOR_BLACK /* Black on green */

#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

#define TRUE 1
#define FALSE 0

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required for Windows"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX 1
#elif defined(__unix__)
#define PLATFORM_UNIX 1
#elif __APPLE__
#define PLATFORM_APPLE 1
#else
#error "Unknown platform"
#endif

#ifdef EXPORT
#ifdef _MSC_VER
#define API __declspec(dllexport)
#else
#define API __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define API __declspec(dllimport)
#else
#define API
#endif
#endif

#endif