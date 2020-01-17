#include <prelude.h>
#include "fs.h"

#include <system.h>

#include "fs/memoryfile.h"

#include "allocator.h"
#include "buffer.h"
#include "misc.h"
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
    Slice(String) delims = Slice_of(String, ((Array(String, 1)) { STR("/") }));
    bool absolute = false;
    Vector(String) parts = Vector_new(String, allocator);
    uint8_t i = 0;
    String state = path;
    for (String head; String_delim(&state, delims, &head); ++i) {
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
    Slice(String) delims = Slice_of(String, ((Array(String, 2)) { STR("\\"), STR("/") }));
    bool absolute = false;
    Vector(String) parts = Vector_new(String, allocator);
    uint8_t i = 0;
    String state = path;
    for (String head; String_delim(&state, delims, &head); ++i) {
        Vector_push(&parts, head);
    }
    Vector_push(&parts, state);
    return (FilePath) { ._data = path, .parts = parts, .absolute = absolute };
}

static String FilePath_to_native_unix(FilePath path, Buffer *buf);

static String FilePath_to_native_win(FilePath path, Buffer *buf);

String FilePath_to_native_(FilePath path, Buffer *buf, bool nix)
{
    return nix ? FilePath_to_native_unix(path, buf) : FilePath_to_native_win(path, buf);
}

static String FilePath_to_native_unix(FilePath path, Buffer *buf)
{
    String delim = STR("/");
    File *f = MemoryFile_new(buf);
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

static String FilePath_to_native_win(FilePath path, Buffer *buf)
{
    String delim = STR("\\");
    File *f = MemoryFile_new(buf);
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
    Vector(String) parts = Vector_new(String, allocator);
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

FilePath FilePath_basename(FilePath self, Allocator *allocator)
{
    Vector(String) parts = Vector_new(String, allocator);
    Slice(String) slice = Vector_toSlice(String, &self.parts);
    String it = *Slice_at(&slice, Slice_size(&slice) - 1);
    Vector_push(&parts, it);
    return (FilePath) {
            ._data = STR(""),
            .parts = parts,
            .absolute = false,
    };
}

void FileSystem_delete(FileSystem *self)
{
    FilePath_delete(&self->root);
}

FileSystem FileSystem_new(FileSystem_class class, void *data, Allocator *allocator)
{
    return (FileSystem) {
            .allocator = allocator,
            .class = class,
            .root = FilePath_from(STR(""), allocator),
            .data = data,
    };
}

void FileSystem_newroot(FileSystem *parent, FilePath root, FileSystem *out)
{
    *out = (FileSystem) {
            .allocator = parent->allocator,
            .class = parent->class,
            .root = root,
            .data = out,
    };
}

File *FileSystem_open(FileSystem *self, FilePath path, String mode)
{
    File *(*func)(void *, FilePath, String) = self->class.open;
    assert(func && "FileSystem implements open");
    return func(self->data, path, mode);
}

File *File_new(File_class class, void *data, FileSystem *owner, Allocator *allocator)
{
    File *ret = new(File, ((File) {
            .owner = owner,
            .allocator = allocator,
            .class = class,
            .data = data,
    }));
    return ret;
}

size_t File_read(File *self, Slice(uint8_t) out)
{
    size_t (*func)(void *, Slice(uint8_t)) = self->class.read;
    assert(func && "File implements read");
    return func(self->data, out);
}

size_t File_write(File *self, Slice(uint8_t) in)
{
    size_t (*func)(void *, Slice(uint8_t)) = self->class.write;
    assert(func && "File implements write");
    return func(self->data, in);
}

size_t File_size(File *self)
{
    size_t (*func)(void *) = self->class.size;
    assert(func && "File implements size");
    return func(self->data);
}

bool File_rewind(File *self)
{
    bool (*func)(void *) = self->class.rewind;
    assert(func && "File implements rewind");
    return func(self->data);
}

bool File_flush(File *self)
{
    bool (*func)(void *) = self->class.flush;
    if (!func) {
        return 0;
    }
    return func(self->data);
}

bool File_close(File *self)
{
    Allocator *allocator = self->allocator;
    bool (*func)(void *) = self->class.close;
    bool ret = !func ? true : func(self->data);
    free(self);
    return ret;
}

bool File_read_all(File *self, Slice(uint8_t) *out, Allocator *allocator)
{
    const size_t len = File_size(self);
    uint8_t *buf = new_arr(uint8_t, len + 1);
    Slice(uint8_t) slice = (Slice(uint8_t)) {._begin.r = buf, ._end = buf + len};
    File_read(self, slice);
    buf[len] = 0;
    *out = slice;
    return true;
}

// misc

fs_dirtoken fs_pushd(FilePath path, Allocator *allocator)
{
    size_t size = 1024;
    native_char_t *cwd = libsystem_getcwd(new_arr(native_char_t, size), size);
    assert(cwd && "cwd is large enough");
    Buffer buf = Buffer_new(allocator);
    native_char_t *s = String_cstr(FilePath_to_native(path, &buf), allocator);
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
