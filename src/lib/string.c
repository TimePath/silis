#include "../system.h"
#include "string.h"

bool str_equals(string_view_t self, string_view_t other) {
    const size_t selfSize = str_size(self);
    const size_t otherSize = str_size(other);
    return selfSize == otherSize && memcmp(self.begin, other.begin, selfSize) == 0;
}

static native_char_t spaces[8 * 4 + 1];

STATIC_INIT(spaces) {
    for (size_t i = 0; i < ARRAY_LEN(spaces) - 1; ++i) {
        spaces[i] = ' ';
    }
}

string_view_t str_indent(size_t n) {
    assert(n < ARRAY_LEN(spaces));
    return (string_view_t) {.begin = (const uint8_t *) spaces, .end = (const uint8_t *) (spaces + n)};
}
