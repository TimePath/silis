#include <system.h>
#include "string.h"

bool String_equals(String self, String other) {
    const size_t selfSize = String_sizeBytes(self);
    const size_t otherSize = String_sizeBytes(other);
    return selfSize == otherSize && memcmp(String_begin(self), String_begin(other), selfSize) == 0;
}

static native_char_t *spaces(size_t n) {
    static native_char_t *spaces = NULL;
    static size_t i = 0;
    for (spaces = realloc(spaces, n + 1); i < n; ++i) {
        spaces[i] = ' ';
    }
    return spaces;
}

String String_indent(size_t n) {
    const uint8_t *p = (const uint8_t *) spaces(n);
    Slice(uint8_t) slice = {p, p + n};
    return String_fromSlice(slice, ENCODING_COMPILER);
}
