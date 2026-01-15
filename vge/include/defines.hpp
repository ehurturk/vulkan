#ifndef DEFINES_H_
#define DEFINES_H_

#include <cstdint>

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

#if defined(BUILD_DEBUG)
#define TRUE_IF_DEBUG true
#else
#define TRUE_IF_DEBUG false
#endif

#if defined(__PLATFORM_WINDOWS__)
#ifndef _WIN64
#error "64-bit is required for Windows"
#endif
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
