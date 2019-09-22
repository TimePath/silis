#pragma once

#include "allocator.h"
#include "macro.h"
#include "slice.h"

#define Vector(T) CAT2(Vector__, T)
#define Vector_instantiate(T) typedef Vector_(T) Vector(T)
#define Vector_(T) \
struct { \
    Allocator *_allocator; \
    size_t _size; \
    T *_data; \
}

#define Vector_new(allocator) { \
    ._allocator = allocator, \
    ._size = 0, \
    ._data = NULL, \
} \
/**/

#define Vector_size(self) ((self)->_size)
#define Vector_at(self, i) (&(self)->_data[i])
#define _Vector_at(T, sizeof_T, self, i) ((T *) (void *) &((uint8_t *) (self)->_data)[(sizeof_T) * (i)])
#define Vector_toSlice(T, self) ((Slice(T)) { ._begin.r = (self)->_data, ._end = (self)->_data + Vector_size(self) })

#define Vector_loop(T, self, i) Slice_loop(&Vector_toSlice(T, self), i)

void _Vector_push(size_t sizeof_T, void *self, size_t dataSize, const void *data, size_t count);

#define Vector_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) ((self)->_data == &(val)); } \
    _Vector_push(sizeof(val), self, sizeof(val), &(val), 1); \
MACRO_END

void Vector_pop(void *self);

#define Vector_delete(T, self) \
MACRO_BEGIN \
Vector_loop(T, self, __i) { \
    T *__it = Vector_at(self, __i); \
    CAT2(T, _delete)(__it); \
} \
_Vector_delete(self); \
MACRO_END

void _Vector_delete(void *self);
