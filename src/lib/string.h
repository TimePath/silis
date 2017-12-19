#pragma once

#include "string/ascii.h"
#include "macro.h"

typedef struct {
    union {
        struct {
            const void *begin;
            /// one after the actual last character (the \0 of a null-terminated string)
            const void *end;
        };
#ifndef NDEBUG
        struct {
            const native_char_t *begin;
            const native_char_t *end;
        } debug;
#endif
    };
} string_view_t;

INLINE string_view_t str_from(const void *begin, const void *end) {
    return (string_view_t) {.begin = begin, .end = end};
}

INLINE const void *str_begin(string_view_t self) {
    return self.begin;
}

INLINE const void *str_end(string_view_t self) {
    return self.end;
}

#define STR_(str) str_from((str), (str) + sizeof (str) - 1)
#define STR(str) STR_(ASCII_(str))

INLINE size_t str_byte_size(string_view_t self) {
    assert(str_end(self) >= str_begin(self));
    return (size_t) ((uint8_t *) str_end(self) - (uint8_t *) str_begin(self));
}

bool str_equals(string_view_t self, string_view_t other);

string_view_t str_indent(size_t n);
