#pragma once

#include "buffer.h"
#include "string.h"

void fprintf_zu(FILE *stream, size_t zu);

void fprintf_buf(FILE *stream, Buffer buf);

void fprintf_s(FILE *stream, String s);
