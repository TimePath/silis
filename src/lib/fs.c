#include <system.h>
#include "fs.h"

#include "allocator.h"
#include "buffer.h"
#include "stdio.h"

void FilePath_delete(FilePath *self)
{
    Vector_delete(String, &self->parts);
}

static FilePath FilePath_from_native_unix(String path, Allocator *allocator);

static FilePath FilePath_from_native_win(String path, Allocator *allocator);

FilePath FilePath_from(String path, Allocator *allocator)
{
    return FilePath_from_native_unix(path, allocator);
}

FilePath FilePath_from_native_(String path, bool nix, Allocator *allocator)
{
    return nix ? FilePath_from_native_unix(path, allocator) : FilePath_from_native_win(path, allocator);
}

static FilePath FilePath_from_native_unix(String path, Allocator *allocator)
{
    String delim = STR("/");
    bool absolute = false;
    Vector(String) parts = Vector_new(allocator);
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

static FilePath FilePath_from_native_win(String path, Allocator *allocator)
{
    String delim = STR("\\");
    bool absolute = false;
    Vector(String) parts = Vector_new(allocator);
    uint8_t i = 0;
    String state = path;
    for (String head; String_delim(&state, delim, &head); ++i) {
        Vector_push(&parts, head);
    }
    Vector_push(&parts, state);
    return (FilePath) { ._data = path, .parts = parts, .absolute = absolute };
}

static String FilePath_to_native_unix(FilePath path, Buffer *buf, Allocator *allocator);

static String FilePath_to_native_win(FilePath path, Buffer *buf, Allocator *allocator);

String FilePath_to_native_(FilePath path, Buffer *buf, bool nix, Allocator *allocator)
{
    return nix ? FilePath_to_native_unix(path, buf, allocator) : FilePath_to_native_win(path, buf, allocator);
}

static String FilePath_to_native_unix(FilePath path, Buffer *buf, Allocator *allocator)
{
    String delim = STR("/");
    File *f = Buffer_asFile(buf, allocator);
    Vector_loop(String, &path.parts, i) {
        String it = *Vector_at(&path.parts, i);
        if (i == 0 ? path.absolute : true) {
            fprintf_s(f, delim);
        }
        fprintf_s(f, it);
    }
    File_close(f);
    String ret = String_fromSlice(Vector_toSlice(uint8_t, buf), ENCODING_DEFAULT);
    return ret;
}

static String FilePath_to_native_win(FilePath path, Buffer *buf, Allocator *allocator)
{
    String delim = STR("\\");
    File *f = Buffer_asFile(buf, allocator);
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
    File_close(f);
    String ret = String_fromSlice(Vector_toSlice(uint8_t, buf), ENCODING_DEFAULT);
    return ret;
}

FilePath FilePath_dirname(FilePath self, Allocator *allocator)
{
    Vector(String) parts = Vector_new(allocator);
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

void FileSystem_delete(FileSystem *self)
{
    FilePath_delete(&self->root);
}

FileSystem FileSystem_new(FilePath root)
{
    return (FileSystem) {
            .root = root,
    };
}

static File *fs_open(FileSystem *fs, FilePath path, String mode, Allocator *allocator);

File *FileSystem_open(FileSystem *fs, FilePath path, String mode, Allocator *allocator)
{
    return fs_open(fs, path, mode, allocator);
}

File *File_new(File_class class, void *self, Allocator *allocator)
{
    File *ret = malloc(sizeof(*ret));
    *ret = (File) {
            .allocator = allocator,
            .class = class,
            .self = self,
    };
    return ret;
}

ssize_t File_read(File *self, Slice(uint8_t) out)
{
    ssize_t (*func)(void *, Slice(uint8_t)) = self->class.read;
    assert(func && "File implements read");
    return func(self->self, out);
}

ssize_t File_write(File *self, Slice(uint8_t) in)
{
    ssize_t (*func)(void *, Slice(uint8_t)) = self->class.write;
    assert(func && "File implements write");
    return func(self->self, in);
}

ssize_t File_seek(File *self, off64_t pos, uint8_t whence)
{
    ssize_t (*func)(void *, off64_t pos, uint8_t whence) = self->class.seek;
    assert(func && "File implements seek");
    return func(self->self, pos, whence);
}

ssize_t File_tell(File *self)
{
    ssize_t (*func)(void *) = self->class.tell;
    assert(func && "File implements tell");
    return func(self->self);
}

ssize_t File_flush(File *self)
{
    ssize_t (*func)(void *) = self->class.flush;
    if (!func) {
        return 0;
    }
    return func(self->self);
}

ssize_t File_close(File *self)
{
    Allocator *allocator = self->allocator;
    ssize_t (*func)(void *) = self->class.close;
    ssize_t ret = !func ? 0 : func(self->self);
    free(self);
    return ret;
}

bool File_read_all(File *self, Slice(uint8_t) *out, Allocator *allocator)
{
    File_seek(self, 0, File_seek_end);
    ssize_t ret = File_tell(self);
    if (ret < 0) {
        return false;
    }
    const size_t len = (size_t) ret;
    File_seek(self, 0, File_seek_begin);
    uint8_t *buf = malloc(len + 1);
    Slice(uint8_t) slice = (Slice(uint8_t)) {._begin.r = buf, ._end = buf + len};
    File_read(self, slice);
    buf[len] = 0;
    *out = slice;
    return true;
}

// impl: native

static ssize_t File_native_read(void *self, Slice(uint8_t) out)
{
    return (ssize_t) libsystem_fread(Slice_data_mut(&out), sizeof(uint8_t), Slice_size(&out), self);
}

static ssize_t File_native_write(void *self, Slice(uint8_t) in)
{
    return (ssize_t) libsystem_fwrite(_Slice_data(&in), sizeof(uint8_t), Slice_size(&in), self);
}

static ssize_t File_native_seek(void *self, off64_t pos, uint8_t whence)
{
    uint8_t w = whence == File_seek_begin ? libsystem_SEEK_SET : libsystem_SEEK_END;
    return libsystem_fseek(self, (native_long_t) pos, w);
}

static ssize_t File_native_tell(void *self)
{
    return libsystem_ftell(self);
}

static ssize_t File_native_flush(void *self)
{
    return libsystem_fflush(self);
}

static ssize_t File_native_close(void *self)
{
    return libsystem_fclose(self);
}

static File_class File_native = {
    .read = File_native_read,
    .write = File_native_write,
    .seek = File_native_seek,
    .tell = File_native_tell,
    .flush = File_native_flush,
    .close = File_native_close,
};

static File *fs_open(FileSystem *fs, FilePath path, String mode, Allocator *allocator)
{
    Buffer buf = Buffer_new(allocator);
    FilePath_to_native(fs->root, &buf, allocator);
    String slash = STR("/");
    Vector_push(&buf, *Slice_at(&slash.bytes, 0));
    native_char_t *p = String_cstr(FilePath_to_native(path, &buf, allocator), allocator);
    Buffer_delete(&buf);
    native_char_t *m = String_cstr(mode, allocator);
    libsystem_FILE *file = libsystem_fopen(p, m);
    assert(file && "File exists");
    free(p);
    free(m);
    return File_new(File_native, file, allocator);
}

// misc

fs_dirtoken fs_pushd(FilePath path, Allocator *allocator)
{
    size_t size = 1024;
    native_char_t *cwd = libsystem_getcwd(malloc(size), size);
    assert(cwd && "cwd is large enough");
    Buffer buf = Buffer_new(allocator);
    native_char_t *s = String_cstr(FilePath_to_native(path, &buf, allocator), allocator);
    Buffer_delete(&buf);
    libsystem_chdir(s);
    free(s);
    return (fs_dirtoken) {allocator, cwd};
}

void fs_popd(fs_dirtoken tok)
{
    Allocator *allocator = tok.allocator;
    native_char_t *s = tok.prev;
    libsystem_chdir(s);
    free(s);
}

File *fs_stdout(Allocator *allocator)
{
    static File *ret = NULL;
    if (!ret) ret = File_new(File_native, libsystem_stdout(), allocator);
    return ret;
}
