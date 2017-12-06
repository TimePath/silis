#include "../system.h"
#include "stdio.h"

void fprintf_s(FILE *stream, string_view_t s) {
    fwrite(s.begin, sizeof(char), str_size(s), stream);
}

void fprintf_buf(FILE *stream, buffer_t buf) {
    fprintf_s(stream, (string_view_t) {.begin = buf.data, .end = buf.data + buf.size});
}

typedef size_t itoa_T;

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
static char itoa_buf[((size_t) (8 * sizeof(itoa_T) / 2) + 3)];

static const char itoa_lookup[] =
        /**/    "0001020304050607080910111213141516171819"
                "2021222324252627282930313233343536373839"
                "4041424344454647484950515253545556575859"
                "6061626364656667686970717273747576777879"
                "8081828384858687888990919293949596979899";

static string_view_t itoa(itoa_T val) {
    char *end = &itoa_buf[ARRAY_LEN(itoa_buf) - 1];
    char *p = end;
    while (val >= 100) {
        const itoa_T index = (val % 100) * 2;
        val /= 100;
        *--p = itoa_lookup[index + 1];
        *--p = itoa_lookup[index];
    }
    const itoa_T index = val * 2;
    *--p = itoa_lookup[index + 1];
    *--p = itoa_lookup[index];
    char *begin = &p[val < 10];
    return (string_view_t) {.begin = begin, .end = end};
}

void fprintf_zu(FILE *stream, size_t zu) {
    fprintf_s(stream, itoa(zu));
}
