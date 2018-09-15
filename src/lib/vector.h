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

#define Vector_size(self) ((self)->_size)
#define Vector_data(self) ((self)->_data)
#define Vector_toSlice(T, self) ((Slice(T)) { Vector_data(self), Vector_data(self) + Vector_size(self) })

void (Vector_push)(size_t sizeof_T, void *self, size_t dataSize, const void *data);

#define Vector_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) (Vector_data(self) == &(val)); } \
    Vector_push(sizeof(val), self, sizeof(val), &(val)); \
MACRO_END

void Vector_pop(void *self);

void Vector_delete(void *self);
