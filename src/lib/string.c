#include <system.h>
#include "string.h"

bool String_equals(String self, String other)
{
    const size_t selfSize = String_sizeBytes(self);
    const size_t otherSize = String_sizeBytes(other);
    return selfSize == otherSize && memcmp(String_begin(self), String_begin(other), selfSize) == 0;
}

static native_char_t *spaces(size_t n)
{
	static native_char_t *spaces = NULL;
	static size_t i = 0;
	static size_t biggest = 0;
	if (n > biggest) {
        biggest = n;
        spaces = realloc(spaces, n + 1);
        for (; i < n; ++i) {
            spaces[i] = ' ';
        }
        spaces[i] = 0;
	}
	return spaces;
}

String String_indent(size_t n)
{
    const uint8_t *p = (const uint8_t *) spaces(n);
    Slice(uint8_t) slice = {._begin = p, ._end = p + n};
    return String_fromSlice(slice, ENCODING_COMPILER);
}

native_char_t *String_cstr(String self)
{
    size_t n = String_sizeBytes(self);
    native_char_t *buf = realloc(NULL, n + 1);
    memcpy(buf, String_begin(self), n);
    buf[n] = 0;
    return buf;
}

bool String_delim(String *tail, String delim, String *head)
{
    const StringEncoding *enc = tail->encoding;
    const uint8_t *begin = String_begin(*tail);
    Slice(uint8_t) it = tail->bytes;
    for (Slice(uint8_t) next; (void) (next = enc->next(it)), Slice_begin(&it) != Slice_end(&it); it = next) {
        size_t c = enc->get(it);
        if (c == Slice_data(&delim.bytes)[0]) { // todo: use all of `delim`
            *head = String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}, enc);
            *tail = String_fromSlice(next, enc);
            return true;
        }
    }
    return false;
}
