#pragma once

#include "vector.h"

instantiate_vec_t(uint8_t);
typedef vec_t(uint8_t) buffer_t;

FILE *buf_file(buffer_t *self);
