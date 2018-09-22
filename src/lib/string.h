#pragma once

#include "macro.h"
#include "slice.h"

typedef struct {
    native_string_t name;
    size_t size_point;
    size_t size_unit;

    /** a single codepoint may encode to a variable amount of codeunits */
    size_t (*point_units)(size_t codepoint);

    /** how many codepoints are in this string? */
    size_t (*count_points)(Slice(uint8_t) slice);

    /** how many codeunits does the entire string encode to? */
    size_t (*count_units)(Slice(uint8_t) slice);
    /** increment by 1 codepoint */
    Slice(uint8_t) (*next)(Slice(uint8_t) slice);
    /** increment by n codeunits */
    Slice(uint8_t) (*skip_units)(Slice(uint8_t) slice, size_t n);

    /** get the first codepoint */
    size_t (*get)(Slice(uint8_t) slice);
} StringEncoding;

#define ENCODING_DEFAULT ENCODING_ASCII

extern StringEncoding _ENCODING_ASCII;
#define ENCODING_ASCII &_ENCODING_ASCII

#define ENCODING_SYSTEM ENCODING_ASCII
#define ENCODING_COMPILER ENCODING_ASCII

typedef struct {
    const StringEncoding *encoding;
    Slice(uint8_t) bytes;
} String;

#define STR(str) STR_(""str"")
#define STR_(str) String_fromSlice(CAST(Slice(uint8_t), Slice(void), _Slice_of((str), (sizeof (str) - 1))), ENCODING_COMPILER)

INLINE String String_fromSlice(Slice(uint8_t) slice, const StringEncoding *encoding)
{
    return (String) {.bytes = slice, .encoding = encoding};
}

INLINE const void *String_begin(String self)
{
    return Slice_begin(&self.bytes);
}

INLINE const void *String_end(String self)
{
    return Slice_end(&self.bytes);
}

INLINE size_t String_sizePoints(String self)
{
    return self.encoding->count_points(self.bytes);
}

INLINE size_t String_sizeUnits(String self)
{
    return self.encoding->count_units(self.bytes);
}

INLINE size_t String_sizeBytes(String self)
{
    return String_sizeUnits(self) * self.encoding->size_unit;
}

bool String_equals(String self, String other);

String String_indent(size_t n);
