#include "../../system.h"
#include "ascii.h"

struct ascii_unit {
    uint8_t it;
};

static size_t ascii_codepoint_size(ascii_codepoint c) {
    (void) c;
    return 1;
}

const ascii_unit *ascii_next(const ascii_unit *buf) {
    return buf + 1;
}

size_t ascii_unit_count(const ascii_unit *begin, const ascii_unit *end) {
    assert(end >= begin);
    return end - begin;
}

const ascii_unit *ascii_unit_skip(const ascii_unit *buf, size_t n) {
    return buf + n;
}

ascii_codepoint ascii_get(const ascii_unit *buf) {
    return buf->it;
}

ascii_unit *ascii_set(ascii_unit *buf, ascii_codepoint c) {
    buf->it = (uint8_t) c;
    return buf + ascii_codepoint_size(c);
}
