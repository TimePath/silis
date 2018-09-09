#pragma once

#include "macro.h"

#define Slice(T) Slice__##T
#define Slice_$(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    const T *_begin; \
    const T *_end; /** one after the actual last element */ \
}

Slice_$(uint8_t);

#define Slice_begin(self) ((self)->_begin)
#define Slice_end(self) ((self)->_end)

#define Slice_size(self) ((size_t) ((self)->_end - (self)->_begin))
#define Slice_data(self) ((self)->_begin)

#define Slice_loop(self, i) for (size_t n = Slice_size(self), (i) = 0; (i) < n; ++(i))
