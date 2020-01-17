#pragma once

#include <_prelude/probe.h>

#if TARGET_SIZEOF_POINTER == 64
typedef uint64_t size_t;
#elif TARGET_SIZEOF_POINTER == 32
typedef uint32_t size_t;
#else
#error "Unknown size_t"
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif
