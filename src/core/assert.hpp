#ifndef ASSERT_H_
#define ASSERT_H_

#include "defines.hpp"
#include <string_view>
#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugBreak()
#else
#define debugBreak() __building_trap()
#endif

API void assertion_report_failure(std::string_view expr, std::string_view msg, std::string_view file, i32 line);

#define ASSERT_MSG(expr, msg)                                         \
    {                                                                 \
        if (expr) {                                                   \
        } else {                                                      \
            assertion_report_failure(#expr, msg, __FILE__, __LINE__); \
        }                                                             \
    }

#define ASSERT(expr) ASSERT_MSG(expr, "")

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr)
#endif

#endif
