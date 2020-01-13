#pragma once

#include <_prelude/probe.h>

#ifndef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

#if TARGET_OS == OS_MACOS
#undef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

#if !defined(static_assert)
#define static_assert(expr, message) extern char (*ct_assert(void)) [sizeof(char[1 - 2*!(expr)])]
#endif

// region __WORDSIZE

#if !USE_REAL_HEADERS
#ifndef __WORDSIZE
#if defined(__LP64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)
#define __WORDSIZE 64
#elif defined(__ILP32__) || defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM)
#define __WORDSIZE 32
#endif
#endif
#endif

// endregion

// region stdint

#if USE_REAL_HEADERS
#include <stdint.h>
#else

typedef unsigned char uint8_t;
static_assert(sizeof(uint8_t) * 8 == 8, "uint8_t");

typedef unsigned short uint16_t;
static_assert(sizeof(uint16_t) * 8 == 16, "uint16_t");

typedef unsigned int uint32_t;
typedef int int32_t;
static_assert(sizeof(uint32_t) * 8 == 32, "uint32_t");
static_assert(sizeof(int32_t) * 8 == 32, "int32_t");

#if __WORDSIZE == 64
typedef unsigned long uint64_t;
typedef long int64_t;
#elif  __WORDSIZE == 32
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
#error "Unknown  __WORDSIZE"
#endif
static_assert(sizeof(uint64_t) * 8 == 64, "uint64_t");
static_assert(sizeof(int64_t) * 8 == 64, "int64_t");

#endif

// endregion

// region stdbool

#if USE_REAL_HEADERS
#include <stdbool.h>
#else

#define bool _Bool
#define true 1
#define false 0

#endif

// endregion

// region stddef

#if USE_REAL_HEADERS
#include <stddef.h>
#else

#if __WORDSIZE == 64
typedef uint64_t size_t;
#elif  __WORDSIZE == 32
typedef uint32_t size_t;
#else
#error "Unknown  __WORDSIZE"
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif

// endregion

// region hide builtin type keywords

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

// endregion

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
