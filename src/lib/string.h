#pragma once

#include "string/ascii.h"
#include "macro.h"
#include "slice.h"

Slice_$(void);
#ifndef NDEBUG
Slice_$(native_char_t);
#endif

typedef struct {
    union {
        Slice(void) slice;
#ifndef NDEBUG
        Slice(native_char_t) debug;
#endif
    };
} String;

#define STR(str) STR_(ASCII_(str))
#define STR_(str) String_fromSlice((Slice(void)) {(str), (str) + sizeof (str) - 1})

INLINE String String_fromSlice(Slice(void) slice) {
    return (String) {.slice = slice};
}

INLINE const void *String_begin(String self) {
    return self.slice.begin;
}

INLINE const void *String_end(String self) {
    return self.slice.end;
}

INLINE size_t String_sizeBytes(String self) {
    assert(String_end(self) >= String_begin(self));
    return (size_t) ((uint8_t *) String_end(self) - (uint8_t *) String_begin(self));
}

bool String_equals(String self, String other);

String String_indent(size_t n);
