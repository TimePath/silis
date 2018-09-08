#pragma once

#include "macro.h"

#define Slice(T) Slice__##T
#define Slice_$(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    const T *begin; \
    const T *end; /** one after the actual last element */ \
}

#define Slice_loop(self, i) for (size_t n = (self).end - (self).begin, (i) = 0; (i) < n; ++(i))
