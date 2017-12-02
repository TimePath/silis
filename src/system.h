#pragma once

#ifndef __WORDSIZE
#if defined __x86_64__ && !defined __ILP32__
#define __WORDSIZE 64
#else
#define __WORDSIZE	32
#endif
#endif

//#include <stdbool.h>

#define bool _Bool
#define true 1
#define false 0

//#include <stdint.h>

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

//#include <stddef.h>

#if __WORDSIZE == 64
typedef uint64_t size_t;
#elif  __WORDSIZE == 32
typedef uint32_t size_t;
#else
#error "Unknown  __WORDSIZE"
#endif

#define NULL ((void *) 0)

//#include <stdio.h>

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

extern int fclose(FILE *stream);

extern int fprintf(FILE *stream, const char *format, ...);

extern FILE *stdout;

//#include <stdlib.h>

enum {
    EXIT_SUCCESS = 0,
};

extern unsigned long strtoul(const char *nptr, char **endptr, int base);

extern void free(void *ptr);

extern void *realloc(void *ptr, size_t size);

//#include <string.h>

extern int memcmp(const void *s1, const void *s2, size_t n);

extern void *memcpy(void *dest, const void *src, size_t n);

extern size_t strlen(const char *s);

#include <assert.h>
