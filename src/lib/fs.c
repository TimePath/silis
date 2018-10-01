#include <system.h>
#include "fs.h"

static fs_dirtoken _fs_pushd(native_char_t *path);

fs_dirtoken fs_pushd_dirname(String path)
{
    native_char_t *s = String_cstr(path);
    fs_dirtoken ret = _fs_pushd(dirname(s));
    free(s);
    return ret;
}

fs_dirtoken fs_pushd(String path)
{
    native_char_t *s = String_cstr(path);
    fs_dirtoken ret = _fs_pushd(s);
    free(s);
    return ret;
}

static fs_dirtoken _fs_pushd(native_char_t *path)
{
    size_t size = 1024;
    native_char_t *ret = getcwd(realloc(NULL, size), size);
    assert(ret && "buf is large enough");
    chdir(path);
    return (fs_dirtoken) {ret};
}

void fs_popd(fs_dirtoken tok)
{
    chdir(tok.prev);
    free(tok.prev);
}

bool fs_read(String path, String *out)
{
    FILE *file = fopen(String_cstr(path), "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    const native_long_t ret = ftell(file);
    if (ret < 0) {
        return false;
    }
    const size_t len = (size_t) ret;
    fseek(file, 0, SEEK_SET);
    uint8_t *buf = realloc(NULL, len + 1);
    size_t read = fread(buf, len, 1, file);
    (void) read;
    buf[len] = 0;
    fclose(file);
    *out = String_fromSlice((Slice(uint8_t)) {._begin = buf, ._end = buf + len}, ENCODING_DEFAULT);
    return true;
}
