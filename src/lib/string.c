#include <system.h>
#include "string.h"

#include "allocator.h"

bool String_equals(String self, String other)
{
    const size_t selfSize = String_sizeBytes(self);
    const size_t otherSize = String_sizeBytes(other);
    return selfSize == otherSize && memcmp(String_begin(self), String_begin(other), selfSize) == 0;
}

static native_char_t *spaces(size_t n, Allocator *allocator)
{
    static native_char_t *_spaces = NULL;
	static size_t i = 0;
	static size_t biggest = 0;
    if (n == 0) {
        i = 0;
        biggest = 0;
        free(_spaces);
        return (_spaces = NULL);
    }
	if (n > biggest) {
        biggest = n;
        _spaces = realloc(_spaces, n + 1);
        for (; i < n; ++i) {
            _spaces[i] = ' ';
        }
        _spaces[i] = 0;
	}
	return _spaces;
}

String String_indent(size_t n, Allocator *allocator)
{
    const uint8_t *p = (const uint8_t *) spaces(n, allocator);
    Slice(uint8_t) slice = {._begin.r = p, ._end = p + n};
    return String_fromSlice(slice, ENCODING_COMPILER);
}

native_char_t *String_cstr(String self, Allocator *allocator)
{
    size_t n = String_sizeBytes(self);
    native_char_t *buf = new_arr(native_char_t, n + 1);
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
        if (c == *Slice_at(&delim.bytes, 0)) { // todo: use all of `delim`
            *head = String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc);
            *tail = String_fromSlice(next, enc);
            return true;
        }
    }
    return false;
}
