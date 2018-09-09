#pragma once

#include "vector.h"

Vector_$(uint8_t);
typedef Vector(uint8_t) Buffer;

#define Buffer_toSlice(self) Vector_toSlice(uint8_t, (self))

FILE *Buffer_asFile(Buffer *self);
