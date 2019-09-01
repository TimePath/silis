#include <prelude.h>
#include "misc.h"

#include <system.h>

void _assert(bool ok, native_string_t expr, native_string_t file, native_string_t line, native_string_t func)
{
    if (ok) return;
    libsystem_assert_fail(expr, file, line, func);
}

ssize_t _memcmp(const void *s1, const void *s2, size_t n) {
    return libsystem_memcmp(s1, s2, n);
}

void *_memcpy(void *dest, const void *src, size_t n) {
    return libsystem_memcpy(dest, src, n);
}
