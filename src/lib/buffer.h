#pragma once

#include "allocator.h"
#include "vector.h"

Vector_instantiate(uint8_t);
typedef Vector(uint8_t) Buffer;

#define uint8_t_delete(self) ((void) (self))

#define Buffer_new(allocator) ((Buffer) Vector_new(allocator))

#define Buffer_delete(self) Vector_delete(uint8_t, self)

#define Buffer_toSlice(self) Vector_toSlice(uint8_t, (self))
