#pragma once

#include "vector.h"

Vector_instantiate(uint8_t);
typedef Vector(uint8_t) Buffer;

#define uint8_t_delete(self) ((void) (self))

#define Buffer_new() (Buffer) Vector_new()

#define Buffer_delete(self) Vector_delete(uint8_t, self)

#define Buffer_toSlice(self) Vector_toSlice(uint8_t, (self))

struct File_s *Buffer_asFile(Buffer *self);
