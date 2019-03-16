#include <system.h>
#include "fs.h"

File *fs_open_(File_class class, void *self)
{
    File *ret = realloc(NULL, sizeof(*ret));
    *ret = (File) {
            .class = class,
            .self = self,
    };
    return ret;
}

static File *fs_open_native(String path, String mode);

File *fs_open(String path, String mode)
{
    return fs_open_native(path, mode);
}

ssize_t fs_read(File *self, Slice(uint8_t) out)
{
    ssize_t (*func)(void *, Slice(uint8_t)) = self->class.read;
    assert(func && "File implements read");
    return func(self->self, out);
}

ssize_t fs_write(File *self, Slice(uint8_t) in)
{
    ssize_t (*func)(void *, Slice(uint8_t)) = self->class.write;
    assert(func && "File implements write");
    return func(self->self, in);
}

ssize_t fs_seek(File *self, off64_t pos, uint8_t whence)
{
    ssize_t (*func)(void *, off64_t pos, uint8_t whence) = self->class.seek;
    assert(func && "File implements seek");
    return func(self->self, pos, whence);
}

ssize_t fs_tell(File *self)
{
    ssize_t (*func)(void *) = self->class.tell;
    assert(func && "File implements tell");
    return func(self->self);
}

ssize_t fs_flush(File *self)
{
    ssize_t (*func)(void *) = self->class.flush;
    if (!func) {
        return 0;
    }
    return func(self->self);
}

ssize_t fs_close(File *self)
{
    ssize_t (*func)(void *) = self->class.close;
    ssize_t ret = !func ? 0 : func(self->self);
    free(self);
    return ret;
}

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

uint8_t *fs_read_all(String path, String *out)
{
    File *file = fs_open(path, STR("rb"));
    if (!file) {
        return false;
    }
    fs_seek(file, 0, SEEK_END);
    ssize_t ret = fs_tell(file);
    if (ret < 0) {
        return false;
    }
    const size_t len = (size_t) ret;
    fs_seek(file, 0, SEEK_SET);
    uint8_t *buf = realloc(NULL, len + 1);
    Slice(uint8_t) slice = (Slice(uint8_t)) {._begin = buf, ._end = buf + len};
    fs_read(file, slice);
    buf[len] = 0;
    fs_close(file);
    *out = String_fromSlice(slice, ENCODING_DEFAULT);
    return buf;
}

// impl: native

static ssize_t File_native_read(void *self, Slice(uint8_t) out)
{
    return (ssize_t) fread(Slice_data_mut(&out), sizeof(uint8_t), Slice_size(&out), self);
}

static ssize_t File_native_write(void *self, Slice(uint8_t) in)
{
    return (ssize_t) fwrite(Slice_data(&in), sizeof(uint8_t), Slice_size(&in), self);
}

static ssize_t File_native_seek(void *self, off64_t pos, uint8_t whence)
{
    return fseek(self, pos, whence);
}

static ssize_t File_native_tell(void *self)
{
    return ftell(self);
}

static ssize_t File_native_flush(void *self)
{
    return fflush(self);
}

static ssize_t File_native_close(void *self)
{
    return fclose(self);
}

static File_class File_native = {
    .read = File_native_read,
    .write = File_native_write,
    .seek = File_native_seek,
    .tell = File_native_tell,
    .flush = File_native_flush,
    .close = File_native_close,
};

File *fs_stdout(void)
{
    static File *ret = NULL;
    if (!ret) ret = fs_open_(File_native, stdout);
    return ret;
}

static File *fs_open_native(String path, String mode)
{
    native_char_t *p = String_cstr(path);
    native_char_t *m = String_cstr(mode);
    FILE *file = fopen(p, m);
    free(p);
    free(m);
    return fs_open_(File_native, file);
}
