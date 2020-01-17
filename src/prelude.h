#pragma once

#ifndef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

#include <_prelude/probe.h>

#if TARGET_OS == OS_MACOS
#undef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

typedef void headers_begin_prelude;
#if USE_REAL_HEADERS

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#else

#include <_prelude/stdint.h>
#include <_prelude/stdbool.h>
#include <_prelude/stddef.h>

#endif
typedef void headers_end_prelude;

typedef char native_char_t;
typedef unsigned char native_uchar_t;
#define char __DO_NOT_USE__

typedef short native_short_t;
typedef unsigned short native_ushort_t;
#define short __DO_NOT_USE__

typedef int native_int_t;
typedef unsigned int native_uint_t;
#define int __DO_NOT_USE__

typedef long native_long_t;
typedef unsigned long native_ulong_t;
#define long __DO_NOT_USE__

typedef const native_char_t *native_string_t;

#define main(...) silis_main(__VA_ARGS__)

#ifdef NDEBUG
#define assert(expr)
#else
#define assert(expr) _assert(expr, #expr, __FILE__, assert_s(__LINE__), __func__)
#define assert_s(s) assert_s2(s)
#define assert_s2(s) #s
#endif

#define malloc(size) Allocator_alloc(allocator, size)
#define realloc(ptr, size) Allocator_realloc(allocator, ptr, size)
#define free(ptr) Allocator_free(allocator, ptr)

#if TARGET_COMPILER == COMPILER_CLANG || TARGET_COMPILER == COMPILER_GCC
#define unreachable(alt) __builtin_unreachable()
#else
#define unreachable(alt) alt
#endif
