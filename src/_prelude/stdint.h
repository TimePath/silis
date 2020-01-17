#pragma once

#include <_prelude/helpers.h>
#include <_prelude/probe.h>

typedef unsigned char uint8_t;
static_assert(sizeof(uint8_t) * 8 == 8, "uint8_t");

typedef unsigned short uint16_t;
static_assert(sizeof(uint16_t) * 8 == 16, "uint16_t");

typedef unsigned int uint32_t;
static_assert(sizeof(uint32_t) * 8 == 32, "uint32_t");

#if TARGET_SIZEOF_LONG == 64
typedef unsigned long uint64_t;
#elif defined(TARGET_SIZEOF_LONGLONG) && TARGET_SIZEOF_LONGLONG == 64
typedef unsigned long long uint64_t;
#else
#error "Unknown uint64_t"
#endif
static_assert(sizeof(uint64_t) * 8 == 64, "uint64_t");
