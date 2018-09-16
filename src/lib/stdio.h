#pragma once

#include "buffer.h"
#include "string.h"

void fprintf_raw(FILE *stream, Slice(uint8_t) slice);

void fprintf_zu(FILE *stream, size_t zu);

void fprintf_s(FILE *stream, String s);

void fprintf_slice(FILE *stream, Slice(uint8_t) slice);

void fprintf_buf(FILE *stream, Buffer *buf);
