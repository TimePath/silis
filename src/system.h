#pragma once

#if defined(__linux__) || defined(__EMSCRIPTEN__)
#define TARGET_OS_WIN 0
#elif defined(_WIN32)
#define TARGET_OS_WIN 1
#else
#error "Unknown OS"
#endif

#define USE_REAL_HEADERS 0

#include <assert.h>

#if defined(__TINYC__)
#define static_assert(expr, message)
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

typedef int64_t off64_t;
#if __WORDSIZE == 64
typedef int64_t off_t;
#elif  __WORDSIZE == 32
typedef int32_t off_t;
#else
#error "Unknown  __WORDSIZE"
#endif

#endif

#if USE_REAL_HEADERS
#include <stddef.h>
#else

#if __WORDSIZE == 64
typedef uint64_t size_t;
typedef int64_t ssize_t;
#elif  __WORDSIZE == 32
typedef uint32_t size_t;
typedef int32_t ssize_t;
#else
#error "Unknown  __WORDSIZE"
#endif

#define NULL ((void *) 0)

#endif

#if USE_REAL_HEADERS
#include <stdio.h>
#else

typedef struct FILE_impl FILE;

extern FILE *fopen(const char *pathname, const char *mode);

enum {
    SEEK_SET = 0,
    SEEK_END = 2,
};

extern int fseek(FILE *stream, long offset, int whence);

extern long ftell(FILE *stream);

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

extern int fflush(FILE *stream);

extern int fclose(FILE *stream);

extern FILE *stdout;

char *getcwd(char *buf, size_t size);

int chdir(const char *path);

#endif

#if USE_REAL_HEADERS
#include <stdlib.h>
#else

enum {
    EXIT_SUCCESS = 0,
};

extern void free(void *ptr);

extern void *realloc(void *ptr, size_t size);

#endif

#if USE_REAL_HEADERS
#include <string.h>
#else

extern int memcmp(const void *s1, const void *s2, size_t n);

extern void *memcpy(void *dest, const void *src, size_t n);

#endif

typedef char native_char_t;
typedef unsigned char native_uchar_t;
#define char __DO_NOT_USE__

typedef const native_char_t *native_string_t;

typedef int native_int_t;
#define int __DO_NOT_USE__

typedef long native_long_t;
#define long __DO_NOT_USE__

#define main(...) _main(__VA_ARGS__)
#define MAIN(impl) \
size_t main(Slice(String) args); \
native_int_t (main)(native_int_t argc, native_string_t argv[]) \
{ \
    extern size_t strlen(native_string_t __s); \
    Vector(String) args = Vector_new(); \
    for (size_t i = 0; i < (size_t) argc; ++i) { \
        native_string_t cstr = argv[i]; \
        Slice(uint8_t) slice = {._begin = (const uint8_t *) cstr, ._end = (const uint8_t *) (cstr + strlen(cstr))}; \
        String s = String_fromSlice(slice, ENCODING_SYSTEM); \
        Vector_push(&args, s); \
    } \
    size_t ret = impl(Vector_toSlice(String, &args)); \
    Vector_delete(String, &args); \
    return (native_int_t) ret; \
}
