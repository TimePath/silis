#pragma once

#include "array.h"
#include "macro.h"

#if TARGET_COMPILER == COMPILER_TCC
#define SLICE_PADDING PADDING(32);
#else
#define SLICE_PADDING
#endif

#define Slice(T) CAT2(Slice__, T)
#define Slice_instantiate(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    union { \
        const T *r; \
        T *w; \
    } _begin; \
    const T *_end; /** one after the actual last element */ \
    SLICE_PADDING \
}

Slice_instantiate(uint8_t);

#define Slice_begin(self) ((self)->_begin.r)
#define Slice_end(self) ((self)->_end)

#define Slice_size(self) ((size_t) (Slice_end(self) - Slice_begin(self)))
#define _Slice_data(self) Slice_begin(self)
#define Slice_at(self, i) (&_Slice_data(self)[i])
#define Slice_data_mut(self) ((self)->_begin.w)

#define Slice_loop(self, i) \
DIAG_PUSH \
DIAG_IGNORE_REDUNDANT_PARENS \
for (size_t CAT2(__n_, __LINE__) = Slice_size(self), (i) = 0; (i) < CAT2(__n_, __LINE__); ++(i)) \
DIAG_POP \
/**/

Slice_instantiate(void);

#define Slice_of(T, expr) _Slice_of_n(T, expr, sizeof(expr))
#define Slice_of_n(T, expr, n) _Slice_of_n(T, expr, sizeof(T) * (n))

#define _Slice_of_n(T, expr, n) CAST(Slice(T), Slice(void), _Slice_of((expr), (n)))

#define __Slice_of(begin, length) ((Slice(void)) { ._begin.r = (begin), ._end = (uint8_t *) (begin) + (length), })

INLINE Slice(void) _Slice_of(void *begin, size_t length)
{
    return __Slice_of(begin, length);
}
