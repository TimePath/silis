#pragma once

#include <lib/macro.h>

NORETURN void libsystem_assert_fail(native_string_t expr, native_string_t file, native_string_t line, native_string_t func);

void *libsystem_malloc(size_t size);

void *libsystem_realloc(void *ptr, size_t size);

void libsystem_free(void *ptr);

native_int_t libsystem_memcmp(const void *s1, const void *s2, size_t n);

void *libsystem_memcpy(void *dest, const void *src, size_t n);

size_t libsystem_strlen(const native_char_t *s);

typedef struct libsystem_FILE libsystem_FILE;

libsystem_FILE *libsystem_fopen(const native_char_t *pathname, const native_char_t *mode);

enum {
    libsystem_SEEK_SET = 0,
    libsystem_SEEK_END = 2,
};

native_int_t libsystem_fseek(libsystem_FILE *stream, native_long_t offset, native_int_t whence);

native_long_t libsystem_ftell(libsystem_FILE *stream);

size_t libsystem_fread(void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream);

size_t libsystem_fwrite(const void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream);

native_int_t libsystem_fflush(libsystem_FILE *stream);

native_int_t libsystem_fclose(libsystem_FILE *stream);

libsystem_FILE *libsystem_stdout(void);

native_char_t *libsystem_getcwd(native_char_t *buf, size_t size);

native_int_t libsystem_chdir(const native_char_t *path);
