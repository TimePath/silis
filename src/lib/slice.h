#pragma once

#include "macro.h"

#define Slice(T) Slice__##T
#define Slice_$(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    const T *begin; \
    const T *end; /** one after the actual last element */ \
}

Slice_$(uint8_t);

#define Slice_size(self) ((size_t) ((self).end - (self).begin))
#define Slice_data(self) ((self).begin)

#define Slice_loop(self, i) for (size_t n = Slice_size(self), (i) = 0; (i) < n; ++(i))
