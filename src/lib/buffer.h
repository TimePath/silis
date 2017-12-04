#pragma once

#include "vector.h"

instantiate_vec_t(char);
typedef vec_t(char) buffer_t;

FILE *buf_file(buffer_t *self);
