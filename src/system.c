#include <lib/macro.h>
#include <stdio.h>

extern char *getcwd(char *buf, size_t size);

extern int chdir(const char *path);

DIAG_PUSH
DIAG_IGNORE("-Wmissing-prototypes")
DIAG_IGNORE("-Wdisabled-macro-expansion") // emscripten stdout macro

typedef struct libsystem_FILE libsystem_FILE;

libsystem_FILE *libsystem_fopen(const char *pathname, const char *mode) {
    return (libsystem_FILE *) fopen(pathname, mode);
}

int libsystem_fseek(libsystem_FILE *stream, long offset, int whence) {
    return fseek((FILE *) stream, offset, whence);
}

long libsystem_ftell(libsystem_FILE *stream) {
    return ftell((FILE *) stream);
}

size_t libsystem_fread(void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream) {
    return fread(ptr, size, nmemb, (FILE *) stream);
}

size_t libsystem_fwrite(const void *ptr, size_t size, size_t nmemb, libsystem_FILE *stream) {
    return fwrite(ptr, size, nmemb, (FILE *) stream);
}

int libsystem_fflush(libsystem_FILE *stream) {
    return fflush((FILE *) stream);
}

int libsystem_fclose(libsystem_FILE *stream) {
    return fclose((FILE *) stream);
}

libsystem_FILE *libsystem_stdout(void) {
    return (libsystem_FILE *) stdout;
}

char *libsystem_getcwd(char *buf, size_t size) {
    return getcwd(buf, size);
}

int libsystem_chdir(const char *path) {
    return chdir(path);
}

DIAG_POP
