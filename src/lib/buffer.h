#pragma once

#include "fs.h"
#include "vector.h"

Vector_instantiate(uint8_t);
typedef Vector(uint8_t) Buffer;

#define Buffer_toSlice(self) Vector_toSlice(uint8_t, (self))

File *Buffer_asFile(Buffer *self);
