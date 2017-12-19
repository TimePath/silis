#pragma once

#include "../macro.h"

typedef native_uchar_t ascii_codepoint;
typedef struct ascii_unit ascii_unit;

#define ASCII_(str) (""str)
#define ASCII(str) ((const ascii_unit *) ASCII_(str))

INLINE const native_char_t *ascii_native(const ascii_unit *buf) {
    return (const native_char_t *) buf;
}

/// increment by 1 codepoint
const ascii_unit *ascii_next(const ascii_unit *buf);

size_t ascii_unit_count(const ascii_unit *begin, const ascii_unit *end);

const ascii_unit *ascii_unit_skip(const ascii_unit *buf, size_t n);

/// get current codepoint value
ascii_codepoint ascii_get(const ascii_unit *buf);

ascii_unit *ascii_set(ascii_unit *buf, ascii_codepoint c);
