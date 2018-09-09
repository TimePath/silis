#include <system.h>

#include "string.h"

bool String_equals(String self, String other) {
    const size_t selfSize = String_sizeBytes(self);
    const size_t otherSize = String_sizeBytes(other);
    return selfSize == otherSize && memcmp(String_begin(self), String_begin(other), selfSize) == 0;
}

static native_char_t spaces[8 * 4 + 1];

STATIC_INIT(spaces) {
    for (size_t i = 0; i < ARRAY_LEN(spaces) - 1; ++i) {
        spaces[i] = ' ';
    }
}

String String_indent(size_t n) {
    assert(n < ARRAY_LEN(spaces));
    Slice(uint8_t) slice = {(const uint8_t *) spaces, (const uint8_t *) (spaces + n)};
    return String_fromSlice(slice, ENCODING_COMPILER);
}
