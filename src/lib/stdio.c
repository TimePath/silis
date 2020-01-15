#include <prelude.h>
#include "stdio.h"

#include "buffer.h"
#include "slice.h"
#include "string.h"

void fprintf_raw(File *stream, Slice(uint8_t) slice)
{
    File_write(stream, slice);
}

typedef size_t itoa_T;

#define itoa silis_itoa

static String itoa(itoa_T val);

void fprintf_zu(File *stream, size_t zu)
{
    fprintf_s(stream, itoa(zu));
}

void fprintf_s(File *stream, String s)
{
    fprintf_raw(stream, s.bytes); // todo: re-encode if needed
}

static uint8_t hexdigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static void _fprintf_hexdump(File *stream, Slice(uint8_t) slice)
{
    for (size_t i = 0; i < Slice_size(&slice); ++i) {
        if (i) { fprintf_s(stream, STR(" ")); }
        const uint8_t b = *Slice_at(&slice, i);
        File_write(stream, Slice_of(uint8_t, ((Array(uint8_t, 2)) {
            hexdigits[((b & 0xF0) >> 4)],
            hexdigits[((b & 0x0F) >> 0)],
        })));
    }
}

void fprintf_slice(File *stream, Slice(uint8_t) slice)
{
    fprintf_s(stream, STR("<Slice "));
    _fprintf_hexdump(stream, slice);
    fprintf_s(stream, STR(">"));
}

void fprintf_buf(File *stream, Buffer *buf)
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
    Slice(uint8_t) slice = (Slice(uint8_t)) {._begin.r = (const uint8_t *) begin, ._end = (const uint8_t *) end};
    return String_fromSlice(slice, ENCODING_COMPILER);
}
