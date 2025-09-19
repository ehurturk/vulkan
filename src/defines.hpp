#ifndef DEFINES_H_
#define DEFINES_H_

#include <string_view>

using uint = unsigned int;

using U8 = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

using I8 = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;

using F32 = float;
using F64 = double;

using B8 = bool;
using B32 = std::int32_t;

namespace Colors {
    inline constexpr std::string_view BLACK = "\033[0;30m";
    inline constexpr std::string_view RED = "\033[0;31m";
    inline constexpr std::string_view GREEN = "\033[0;32m";
    inline constexpr std::string_view YELLOW = "\033[0;33m";
    inline constexpr std::string_view BLUE = "\033[0;34m";
    inline constexpr std::string_view MAGENTA = "\033[0;35m";
    inline constexpr std::string_view CYAN = "\033[0;36m";
    inline constexpr std::string_view WHITE = "\033[0;37m";

    inline constexpr std::string_view BOLD_BLACK = "\033[1;30m";
    inline constexpr std::string_view BOLD_RED = "\033[1;31m";
    inline constexpr std::string_view BOLD_GREEN = "\033[1;32m";
    inline constexpr std::string_view BOLD_YELLOW = "\033[1;33m";
    inline constexpr std::string_view BOLD_BLUE = "\033[1;34m";
    inline constexpr std::string_view BOLD_MAGENTA = "\033[1;35m";
    inline constexpr std::string_view BOLD_CYAN = "\033[1;36m";
    inline constexpr std::string_view BOLD_WHITE = "\033[1;37m";

    namespace Background {
        inline constexpr std::string_view BLACK = "\033[40m";
        inline constexpr std::string_view RED = "\033[41m";
        inline constexpr std::string_view GREEN = "\033[42m";
        inline constexpr std::string_view YELLOW = "\033[43m";
        inline constexpr std::string_view BLUE = "\033[44m";
        inline constexpr std::string_view MAGENTA = "\033[45m";
        inline constexpr std::string_view CYAN = "\033[46m";
        inline constexpr std::string_view WHITE = "\033[47m";
    }

    namespace Format {
        inline constexpr std::string_view RESET = "\033[0m";
        inline constexpr std::string_view BOLD = "\033[1m";
        inline constexpr std::string_view UNDERLINE = "\033[4m";
        inline constexpr std::string_view BLINK = "\033[5m";
        inline constexpr std::string_view REVERSE = "\033[7m";
    }

    inline constexpr std::string_view FATAL = "\033[1;37m\033[41m";
    inline constexpr std::string_view ERROR = "\033[1;31m";
    inline constexpr std::string_view WARN = "\033[1;33m";
    inline constexpr std::string_view INFO = "\033[1;32m";
    inline constexpr std::string_view DEBUG = "\033[1;36m";
    inline constexpr std::string_view TRACE = "\033[1;30m";
}

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
