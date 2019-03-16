#pragma once

#include "buffer.h"
#include "fs.h"
#include "string.h"

void fprintf_raw(File *stream, Slice(uint8_t) slice);

void fprintf_zu(File *stream, size_t zu);

#define fprintf_s fprintf_str

void fprintf_s(File *stream, String s);

void fprintf_slice(File *stream, Slice(uint8_t) slice);

void fprintf_buf(File *stream, Buffer *buf);
