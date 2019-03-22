#pragma once

#include "macro.h"

#define Slice(T) CAT2(Slice__, T)
#define Slice_instantiate(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    union { \
        const T *_begin; \
        T *_begin_mut; \
    }; \
    const T *_end; /** one after the actual last element */ \
}

Slice_instantiate(uint8_t);

#define Slice_begin(self) ((self)->_begin)
#define Slice_end(self) ((self)->_end)

#define Slice_size(self) ((size_t) ((self)->_end - (self)->_begin))
#define Slice_data(self) ((self)->_begin)
#define Slice_data_mut(self) ((self)->_begin_mut)

#define Slice_loop(self, i) \
DIAG_PUSH \
DIAG_IGNORE_REDUNDANT_PARENS \
for (size_t CAT2(__n_, __LINE__) = Slice_size(self), (i) = 0; (i) < CAT2(__n_, __LINE__); ++(i)) \
DIAG_POP \
/**/

Slice_instantiate(void);
#define Slice_of(T, ...) CAST(Slice(T), Slice(void), _Slice_of((__VA_ARGS__), sizeof(__VA_ARGS__)))

INLINE Slice(void) _Slice_of(void *begin, size_t length)
{
    return (Slice(void)) {
            ._begin = begin,
            ._end = (uint8_t *) begin + length,
    };
}
