#pragma once

#define new(T, expr) ((T *) _memcpy(Allocator_alloc(allocator, sizeof(T)), &(expr), sizeof(T)))

#define new_arr(T, n) ((T *) malloc(sizeof(T) * (n)))

void _assert(bool ok, native_string_t expr, native_string_t file, native_string_t line, native_string_t func);

ssize_t _memcmp(const void *s1, const void *s2, size_t n);

void *_memcpy(void *dest, const void *src, size_t n);
