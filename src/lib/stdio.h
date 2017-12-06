#pragma once

#include "string.h"
#include "buffer.h"

void fprintf_s(FILE *stream, string_view_t s);

void fprintf_buf(FILE *stream, buffer_t buf);

void fprintf_zu(FILE *stream, size_t zu);
