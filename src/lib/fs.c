#include <system.h>
#include "fs.h"

#include "buffer.h"
#include "stdio.h"

void FilePath_delete(FilePath *self)
{
    Vector_delete(String, &self->parts);
}

void FileSystem_delete(FileSystem *self)
{
    FilePath_delete(&self->root);
}

FileSystem fs_root(FilePath root)
{
    return (FileSystem) {
            .root = root,
    };
}

FilePath fs_dirname(FilePath self)
{
    Vector(String) parts = Vector_new();
    Slice(String) slice = Vector_toSlice(String, &self.parts);
    slice._end -= 1;
    Slice_loop(&slice, i) {
        String it = *Slice_at(&slice, i);
        Vector_push(&parts, it);
    }
    return (FilePath) {
            ._data = STR(""),
            .parts = parts,
            .absolute = self.absolute,
    };
}

static FilePath fs_path_from_native_unix(String path);

static FilePath fs_path_from_native_win(String path);

FilePath fs_path_from(String path)
{
    return fs_path_from_native_unix(path);
}

FilePath fs_path_from_native_(String path, bool nix)
{
    return nix ? fs_path_from_native_unix(path) : fs_path_from_native_win(path);
}

static FilePath fs_path_from_native_unix(String path)
{
    String delim = STR("/");
    bool absolute = false;
    Vector(String) parts = Vector_new();
    uint8_t i = 0;
    String state = path;
    for (String head; String_delim(&state, delim, &head); ++i) {
        if (!String_sizeBytes(head)) {
            if (i == 0) {
                absolute = true;
            }
            continue;
        }
        Vector_push(&parts, head);
    }
    Vector_push(&parts, state);
    return (FilePath) { ._data = path, .parts = parts, .absolute = absolute };
}

static FilePath fs_path_from_native_win(String path)
{
    String delim = STR("\\");
    bool absolute = false;
    Vector(String) parts = Vector_new();
    uint8_t i = 0;
    String state = path;
    for (String head; String_delim(&state, delim, &head); ++i) {
        Vector_push(&parts, head);
    }
    Vector_push(&parts, state);
    return (FilePath) { ._data = path, .parts = parts, .absolute = absolute };
}

static String fs_path_to_native_unix(FilePath path, Buffer *buf);

static String fs_path_to_native_win(FilePath path, Buffer *buf);

String fs_path_to_native_(FilePath path, Buffer *buf, bool nix)
{
    return nix ? fs_path_to_native_unix(path, buf) : fs_path_to_native_win(path, buf);
}

static String fs_path_to_native_unix(FilePath path, Buffer *buf)
{
    String delim = STR("/");
    File *f = Buffer_asFile(buf);
    Vector_loop(String, &path.parts, i) {
        String it = *Vector_at(&path.parts, i);
        if (i == 0 ? path.absolute : true) {
            fprintf_s(f, delim);
        }
        fprintf_s(f, it);
    }
    fs_close(f);
    String ret = String_fromSlice(Vector_toSlice(uint8_t, buf), ENCODING_DEFAULT);
    return ret;
}

static String fs_path_to_native_win(FilePath path, Buffer *buf)
{
    String delim = STR("\\");
    File *f = Buffer_asFile(buf);
    if (path.absolute) {
        fprintf_s(f, STR("\\\\?\\"));
    }
    Vector_loop(String, &path.parts, i) {
        String it = *Vector_at(&path.parts, i);
        if (i != 0) {
            fprintf_s(f, delim);
        }
        fprintf_s(f, it);
    }
    fs_close(f);
    String ret = String_fromSlice(Vector_toSlice(uint8_t, buf), ENCODING_DEFAULT);
    return ret;
}

File *fs_open_(File_class class, void *self)
{
    File *ret = malloc(sizeof(*ret));
    *ret = (File) {
            .class = class,
            .self = self,
    };
    return ret;
}

static File *fs_open_native(FileSystem *fs, FilePath path, String mode);

File *fs_open(FileSystem *fs, FilePath path, String mode)
{
    return fs_open_native(fs, path, mode);
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

fs_dirtoken fs_pushd(FilePath path)
{
    size_t size = 1024;
    native_char_t *cwd = getcwd(malloc(size), size);
    assert(cwd && "cwd is large enough");
    Buffer buf = Buffer_new();
    native_char_t *s = String_cstr(fs_path_to_native(path, &buf));
    Buffer_delete(&buf);
    chdir(s);
    free(s);
    return (fs_dirtoken) {cwd};
}

void fs_popd(fs_dirtoken tok)
{
    native_char_t *s = tok.prev;
    chdir(s);
    free(s);
}

uint8_t *fs_read_all(FileSystem *fs, FilePath path, String *out)
{
    File *file = fs_open(fs, path, STR("rb"));
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
    uint8_t *buf = malloc(len + 1);
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
    return (ssize_t) fwrite(_Slice_data(&in), sizeof(uint8_t), Slice_size(&in), self);
}

static ssize_t File_native_seek(void *self, off64_t pos, uint8_t whence)
{
    return fseek(self, (native_long_t) pos, whence);
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

static File *fs_open_native(FileSystem *fs, FilePath path, String mode)
{
    Buffer buf = Buffer_new();
    fs_path_to_native(fs->root, &buf);
    String slash = STR("/");
    Vector_push(&buf, *Slice_at(&slash.bytes, 0));
    native_char_t *p = String_cstr(fs_path_to_native(path, &buf));
    Buffer_delete(&buf);
    native_char_t *m = String_cstr(mode);
    FILE *file = fopen(p, m);
    assert(file && "File exists");
    free(p);
    free(m);
    return fs_open_(File_native, file);
}
