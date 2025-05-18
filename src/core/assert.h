#ifndef ASSERT_H_
#define ASSERT_H_

#include "defines.h"
#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugBreak()
#else
#define debugBreak() __building_trap()
#endif

API void report_assertion_failure(const char* expr, const char* msg,
                                  const char* file, i32 line);

#define ASSERT_MSG(expr, msg)                                   \
  {                                                             \
    if (expr) {                                                 \
    } else {                                                    \
      report_assertion_failure(#expr, msg, __FILE__, __LINE__); \
    }                                                           \
  }

#define ASSERT(expr) ASSERT_MSG(expr, "")

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr)
#endif

#endif
