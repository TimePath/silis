#pragma once

#include "macro.h"

#define vec_t_(T) \
struct { \
    size_t size; \
    T *data; \
}

#define vec_t(T) vec_t__##T
#define instantiate_vec_t(T) typedef vec_t_(T) vec_t(T)

void vec_push(void *self, const void *data, size_t size);

#define vec_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) ((self)->data == &(val)); } \
    vec_push(self, &(val), sizeof(val)); \
MACRO_END

void vec_pop(void *self);

void vec_free(void *self);

#define vec_loop(self, i, begin) for (size_t (i) = begin; (i) < (self).size; ++(i))
