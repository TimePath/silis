#pragma once

#include "macro.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    const char *begin;
    /// one after the actual last character (the \0 of a null-terminated string)
    const char *end;
} string_view_t;

#define STR(x) str_new(x)

INLINE string_view_t str_new(const char *str) {
    return (string_view_t) {.begin = str, .end = str + strlen(str)};
}

INLINE size_t str_size(string_view_t self) {
    return self.end - self.begin;
}

bool str_equals(string_view_t self, string_view_t other);

#define STR_PRINTF "%.*s"
#define STR_PRINTF_PASS(self) (int) str_size(self), (self).begin

#define str_loop(self, it, ofs) for (const char *(it) = (self).begin + (ofs); (it) < (self).end; ++(it))
