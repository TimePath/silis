#pragma once

#include "macro.h"

#define Slice(T) CAT2(Slice__, T)
#define Slice_instantiate(T) typedef Slice_(T) Slice(T)
#define Slice_(T) \
struct { \
    const T *_begin; \
    const T *_end; /** one after the actual last element */ \
}

Slice_instantiate(uint8_t);

#define Slice_begin(self) ((self)->_begin)
#define Slice_end(self) ((self)->_end)

#define Slice_size(self) ((size_t) ((self)->_end - (self)->_begin))
#define Slice_data(self) ((self)->_begin)

#define Slice_loop(self, i) \
DIAG_PUSH \
DIAG_IGNORE_REDUNDANT_PARENS \
for (size_t __n = Slice_size(self), (i) = 0; (i) < __n; ++(i)) \
DIAG_POP \
/**/

Slice_instantiate(void);
#define Slice_of(T, ...) CAST(Slice(T), Slice(void), _Slice_of((__VA_ARGS__), sizeof(__VA_ARGS__)))

INLINE Slice(void) _Slice_of(void *begin, size_t length)
{
    return (Slice(void)) {begin, (uint8_t *) begin + length};
}
