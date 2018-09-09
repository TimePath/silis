#pragma once

#define USE_REAL_HEADERS 0

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
_Static_assert(sizeof(uint8_t) * 8 == 8, "uint8_t");

typedef unsigned short uint16_t;
_Static_assert(sizeof(uint16_t) * 8 == 16, "uint16_t");

typedef unsigned int uint32_t;
_Static_assert(sizeof(uint32_t) * 8 == 32, "uint32_t");

#if __WORDSIZE == 64
typedef unsigned long uint64_t;
#elif  __WORDSIZE == 32
typedef unsigned long long uint64_t;
#else
#error "Unknown  __WORDSIZE"
#endif
_Static_assert(sizeof(uint64_t) * 8 == 64, "uint64_t");

#endif

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

#define NULL ((void *) 0)

#endif

#if USE_REAL_HEADERS
#include <stdio.h>
#else

typedef struct FILE_impl FILE;

extern FILE *open_memstream(char **ptr, size_t *sizeloc);

extern FILE *fopen(const char *pathname, const char *mode);

enum {
    SEEK_SET = 0,
    SEEK_END = 2,
};

extern int fseek(FILE *stream, long offset, int whence);

extern long ftell(FILE *stream);

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

extern int fclose(FILE *stream);

extern FILE *stdout;

#endif

#if USE_REAL_HEADERS
#include <stdlib.h>
#else

enum {
    EXIT_SUCCESS = 0,
};

extern unsigned long strtoul(const char *nptr, char **endptr, int base);

extern void free(void *ptr);

extern void *realloc(void *ptr, size_t size);

#endif

#if USE_REAL_HEADERS
#include <string.h>
#else

extern int memcmp(const void *s1, const void *s2, size_t n);

extern void *memcpy(void *dest, const void *src, size_t n);

#endif

#include <assert.h>

typedef char native_char_t;
typedef unsigned char native_uchar_t;
#define char void

typedef const native_char_t *native_string_t;

typedef int native_int_t;
#define int void

typedef long native_long_t;
#define long void

#define main(...) _main(__VA_ARGS__)
#define MAIN(impl) \
size_t main(Vector(String) args); \
native_int_t (main)(native_int_t argc, native_string_t argv[]) \
{ \
    extern size_t strlen(native_string_t __s); \
    Vector(String) args = {0}; \
    for (size_t i = 0; i < (size_t) argc; ++i) { \
        native_string_t cstr = argv[i]; \
        Slice(uint8_t) slice = {(const uint8_t *) cstr, (const uint8_t *) (cstr + strlen(cstr))}; \
        String s = String_fromSlice(slice, ENCODING_SYSTEM); \
        Vector_push(&args, s); \
    } \
    return (native_int_t) impl(args); \
}
