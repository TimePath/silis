#pragma once

#include "macro.h"
#include "slice.h"

#define Vector(T) Vector__##T
#define Vector_$(T) typedef Vector_(T) Vector(T)
#define Vector_(T) \
struct { \
    size_t _size; \
    T *_data; \
}

#define Vector_size(self) ((self)->_size)
#define Vector_data(self) ((self)->_data)
#define Vector_toSlice(T, self) ((Slice(T)) { Vector_data(self), Vector_data(self) + Vector_size(self) })

void Vector_push(void *self, const void *data, size_t size);

#define Vector_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) (Vector_data(self) == &(val)); } \
    Vector_push(self, &(val), sizeof(val)); \
MACRO_END

void Vector_pop(void *self);

void Vector_delete(void *self);
