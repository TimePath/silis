typedef void headers_begin_system;

#include <stdio.h>
#include <stdlib.h>

typedef void headers_end_system;

#include <prelude.h>
#include "system.h"

DIAG_PUSH
DIAG_IGNORE("-Wmissing-prototypes")
DIAG_IGNORE("-Wdisabled-macro-expansion") // emscripten stdout macro

NORETURN void libsystem_assert_fail(native_string_t expr, native_string_t file, native_string_t line, native_string_t func) {
    fputs("Assertion failed (", stderr);
    fputs(file, stderr);
    fputs(":", stderr);
    fputs(line, stderr);
    fputs(" in ", stderr);
    fputs(func, stderr);
    fputs("): ", stderr);
    fputs(expr, stderr);
    fputs("\n", stderr);
    fflush(NULL);
    abort();
}

extern void *(malloc)(size_t size);

void *libsystem_malloc(size_t size) {
    return (malloc)(size);
}

extern void *(realloc)(void *ptr, size_t size);

void *libsystem_realloc(void *ptr, size_t size) {
    return (realloc)(ptr, size);
}

extern void (free)(void *ptr);

void libsystem_free(void *ptr) {
    (free)(ptr);
}

extern native_int_t memcmp(const void *s1, const void *s2, size_t n);

native_int_t libsystem_memcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}

extern void *memcpy(void *dest, const void *src, size_t n);

void *libsystem_memcpy(void *dest, const void *src, size_t n) {
    return memcpy(dest, src, n);
}

extern size_t strlen(const native_char_t *s);

size_t libsystem_strlen(const native_char_t *s) {
    return strlen(s);
}

struct libsystem_FILE *libsystem_fopen(const native_char_t *pathname, const native_char_t *mode) {
    return (struct libsystem_FILE *) fopen(pathname, mode);
}

native_int_t libsystem_fseek(struct libsystem_FILE *stream, native_long_t offset, native_int_t whence) {
    return fseek((FILE *) stream, offset, whence);
}

native_long_t libsystem_ftell(struct libsystem_FILE *stream) {
    return ftell((FILE *) stream);
}

size_t libsystem_fread(void *ptr, size_t size, size_t nmemb, struct libsystem_FILE *stream) {
    return fread(ptr, size, nmemb, (FILE *) stream);
}

size_t libsystem_fwrite(const void *ptr, size_t size, size_t nmemb, struct libsystem_FILE *stream) {
    return fwrite(ptr, size, nmemb, (FILE *) stream);
}

native_int_t libsystem_fflush(struct libsystem_FILE *stream) {
    return fflush((FILE *) stream);
}

native_int_t libsystem_fclose(struct libsystem_FILE *stream) {
    return fclose((FILE *) stream);
}

struct libsystem_FILE *libsystem_stdout(void) {
    return (struct libsystem_FILE *) stdout;
}

extern native_char_t *getcwd(native_char_t *buf, size_t size);

native_char_t *libsystem_getcwd(native_char_t *buf, size_t size) {
    return getcwd(buf, size);
}

extern native_int_t chdir(const native_char_t *path);

native_int_t libsystem_chdir(const native_char_t *path) {
    return chdir(path);
}

DIAG_POP
