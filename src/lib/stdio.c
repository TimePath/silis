#include <system.h>
#include "stdio.h"

#include "buffer.h"
#include "slice.h"
#include "string.h"

void fprintf_raw(FILE *stream, Slice(uint8_t) slice)
{
    fwrite(Slice_begin(&slice), sizeof(uint8_t), Slice_size(&slice), stream);
}

typedef size_t itoa_T;

#define itoa silis_itoa

static String itoa(itoa_T val);

void fprintf_zu(FILE *stream, size_t zu)
{
    fprintf_s(stream, itoa(zu));
}

void fprintf_s(FILE *stream, String s)
{
    fprintf_raw(stream, s.bytes); // todo: re-encode if needed
}

static native_string_t hexdigits = "0123456789abcdef";

static void _fprintf_hexdump(FILE *stream, Slice(uint8_t) slice)
{
    for (size_t i = 0; i < Slice_size(&slice); ++i) {
        if (i) { fprintf_s(stream, STR(" ")); }
        const uint8_t b = Slice_data(&slice)[i];
        fwrite(&hexdigits[((b & 0xF0) >> 4)], sizeof(uint8_t), 1, stream);
        fwrite(&hexdigits[((b & 0x0F) >> 0)], sizeof(uint8_t), 1, stream);
    }
}

void fprintf_slice(FILE *stream, Slice(uint8_t) slice)
{
    fprintf_s(stream, STR("<Slice "));
    _fprintf_hexdump(stream, slice);
    fprintf_s(stream, STR(">"));
}

void fprintf_buf(FILE *stream, Buffer *buf)
{
    fprintf_s(stream, STR("<Buffer "));
    _fprintf_hexdump(stream, Buffer_toSlice(buf));
    fprintf_s(stream, STR(">"));
}

/// buffer size calculated as follows:
/// std::numeric_limits<T>::digits10
/// +1: round up
/// +1: '\0'
/// +1: '+' | '-'
///
/// won't compile:
///
/// #define log10_2 (0.3010299956639812)
/// static char itoa_buf[((size_t) (8 * sizeof(itoa_T) * log10_2) + 3)];
static native_char_t itoa_buf[((size_t) (8 * sizeof(itoa_T) / 2) + 3)];

static native_string_t itoa_lookup =
        /**/    "0001020304050607080910111213141516171819"
                "2021222324252627282930313233343536373839"
                "4041424344454647484950515253545556575859"
                "6061626364656667686970717273747576777879"
                "8081828384858687888990919293949596979899";

static String itoa(itoa_T val)
{
    native_char_t *end = &itoa_buf[ARRAY_LEN(itoa_buf) - 1];
    native_char_t *p = end;
    while (val >= 100) {
        const itoa_T index = (val % 100) * 2;
        val /= 100;
        *--p = itoa_lookup[index + 1];
        *--p = itoa_lookup[index];
    }
    const itoa_T index = val * 2;
    *--p = itoa_lookup[index + 1];
    *--p = itoa_lookup[index];
    native_string_t begin = &p[val < 10];
    Slice(uint8_t) slice = (Slice(uint8_t)) {._begin = (const uint8_t *) begin, ._end = (const uint8_t *) end};
    return String_fromSlice(slice, ENCODING_COMPILER);
}
