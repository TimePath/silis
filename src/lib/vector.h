#pragma once

#include "macro.h"
#include "slice.h"

#define Vector(T) CAT2(Vector__, T)
#define Vector_instantiate(T) typedef Vector_(T) Vector(T)
#define Vector_(T) \
struct { \
    size_t _size; \
    T *_data; \
}

#define Vector_new() { \
    ._size = 0, \
    ._data = NULL, \
} \
/**/

#define Vector_size(self) ((self)->_size)
#define _Vector_data(self) ((self)->_data)
#define Vector_at(self, i) (&_Vector_data(self)[i])
#define Vector_toSlice(T, self) ((Slice(T)) { ._begin = _Vector_data(self), ._end = _Vector_data(self) + Vector_size(self) })

void _Vector_push(size_t sizeof_T, void *self, size_t dataSize, const void *data, size_t count);

#define Vector_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) (_Vector_data(self) == &(val)); } \
    _Vector_push(sizeof(val), self, sizeof(val), &(val), 1); \
MACRO_END

void Vector_pop(void *self);

#define Vector_delete(T, self) \
MACRO_BEGIN \
Slice_loop(&Vector_toSlice(T, self), __i) { \
    T *__it = Vector_at(self, __i); \
    T##_delete(__it); \
} \
_Vector_delete(self); \
MACRO_END

void _Vector_delete(void *self);
