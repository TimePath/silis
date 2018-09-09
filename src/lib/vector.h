#pragma once

#include "macro.h"
#include "slice.h"

#define Vector(T) Vector__##T
#define Vector_$(T) typedef Vector_(T) Vector(T)
#define Vector_(T) \
struct { \
    size_t size; \
    T *data; \
}

#define Vector_toSlice(T, self) ((Slice(T)) { (self).data, (self).data + (self).size })

void Vector_push(void *self, const void *data, size_t size);

#define Vector_push(self, val) \
MACRO_BEGIN \
    if (0) { (void) ((self)->data == &(val)); } \
    Vector_push(self, &(val), sizeof(val)); \
MACRO_END

void Vector_pop(void *self);

void Vector_delete(void *self);
