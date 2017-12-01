#pragma once

#include "vector.h"
#include <stdio.h>

instantiate_vec_t(char);
typedef vec_t(char) buffer_t;

FILE *buf_file(buffer_t *self);

#define BUF_PRINTF "%.*s"
#define BUF_PRINTF_PASS(self) (int) (self).size, (self).data
