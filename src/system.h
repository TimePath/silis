#pragma once

#ifndef USE_REAL_HEADERS
#define USE_REAL_HEADERS 1
#endif

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

#include <assert.h>

#if !defined(static_assert)
#define static_assert(expr, message) extern char (*ct_assert(void)) [sizeof(char[1 - 2*!(expr)])]
#endif

#if !USE_REAL_HEADERS
#ifndef __WORDSIZE
#if defined __x86_64__ && !defined __ILP32__
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif
#endif
#endif

#if USE_REAL_HEADERS
#include <stdbool.h>
#else

#define bool _Bool
#define true 1
#define false 0

#endif

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

#define NULL ((void *) 0)

#endif

#if USE_REAL_HEADERS
#include <string.h>
#else

extern int memcmp(const void *s1, const void *s2, size_t n);

extern void *memcpy(void *dest, const void *src, size_t n);

#endif

typedef struct libsystem_FILE libsystem_FILE;

libsystem_FILE *libsystem_fopen(const char *pathname, const char *mode);

enum {
    libsystem_SEEK_SET = 0,
    libsystem_SEEK_END = 2,
};

int libsystem_fseek(libsystem_FILE *stream, long offset, int whence);

long libsystem_ftell(libsystem_FILE *stream);

size_t libsystem_fread(void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream);

size_t libsystem_fwrite(const void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream);

int libsystem_fflush(libsystem_FILE *stream);

int libsystem_fclose(libsystem_FILE *stream);

libsystem_FILE *libsystem_stdout(void);

char *libsystem_getcwd(char *buf, size_t size);

int libsystem_chdir(const char *path);


typedef char native_char_t;
typedef unsigned char native_uchar_t;
#define char __DO_NOT_USE__

typedef const native_char_t *native_string_t;

typedef int native_int_t;
#define int __DO_NOT_USE__

typedef long native_long_t;
#define long __DO_NOT_USE__

#define unreachable() assert(false)

#define malloc(size) Allocator_alloc(allocator, size)
#define realloc(ptr, size) Allocator_realloc(allocator, ptr, size)
#define free(ptr) Allocator_free(allocator, ptr)

#define MAIN(impl) \
size_t impl(Env env); \
native_int_t main(native_int_t argc, native_string_t argv[]) \
{ \
    size_t ret = Env_run((size_t) argc, argv, impl); \
    return (native_int_t) ret; \
}
