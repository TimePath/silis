#pragma once

#ifndef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

// region __WORDSIZE

#if !USE_REAL_HEADERS
#ifndef __WORDSIZE
#if defined __x86_64__ && !defined __ILP32__
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif
#endif
#endif

// endregion

// region OS

#if 0

#elif defined(__APPLE__)
#define TARGET_OS_WIN 0

#elif defined(__EMSCRIPTEN__)
#define TARGET_OS_WIN 0

#elif defined(__linux__)
#define TARGET_OS_WIN 0

#elif defined(_WIN32)
#define TARGET_OS_WIN 1

#else
#error "Unknown OS"
#endif

// endregion

// region Compiler

#if 0

#elif defined(__clang__)
#define TARGET_COMPILER_CLANG 1
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 0

#elif defined(__GNUC__)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 1
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 0

#elif defined(_MSC_VER)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 1
#define TARGET_COMPILER_TCC 0

#elif defined(__TINYC__)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 1

#else
#error "Unknown compiler"
#endif

// endregion

#if !defined(static_assert)
#define static_assert(expr, message) extern char (*ct_assert(void)) [sizeof(char[1 - 2*!(expr)])]
#endif

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

#if __WORDSIZE == 64
typedef int64_t off_t;
#elif  __WORDSIZE == 32
typedef int32_t off_t;
#else
#error "Unknown  __WORDSIZE"
#endif

#endif

typedef int64_t off64_t;

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
#if !defined(_MSC_VER)
typedef uint64_t size_t;
#endif
typedef int64_t ssize_t;
#elif  __WORDSIZE == 32
#if !defined(_MSC_VER)
typedef uint32_t size_t;
#endif
typedef int32_t ssize_t;
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

#if defined(__TINYC__)
#define unreachable(alt) alt
#else
#define unreachable(alt) __builtin_unreachable()
#endif
