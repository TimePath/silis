#pragma once

#include "macro.h"

typedef struct {
    const uint8_t *begin;
    /// one after the actual last character (the \0 of a null-terminated string)
    const uint8_t *end;
} string_view_t;

#define STR(x) str_new(x)

static INLINE string_view_t str_new(const native_char_t *str) {
    return (string_view_t) {.begin = (const uint8_t *) str, .end = (const uint8_t *) (str + strlen(str))};
}

INLINE size_t str_size(string_view_t self) {
    assert(self.end >= self.begin);
    return (size_t) (self.end - self.begin);
}

bool str_equals(string_view_t self, string_view_t other);

#define str_loop(self, it, ofs) for (const uint8_t *(it) = (self).begin + (ofs); (it) < (self).end; ++(it))

string_view_t str_indent(size_t n);
