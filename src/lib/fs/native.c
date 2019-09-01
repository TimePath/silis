#include <prelude.h>
#include "native.h"

#include <system.h>

#include "../fs.h"
#include "../misc.h"
#include "../slice.h"

extern File_class File_native;

extern FileSystem_class FileSystem_native;

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

File_class File_native = {
        .read = File_native_read,
        .write = File_native_write,
        .seek = File_native_seek,
        .tell = File_native_tell,
        .flush = File_native_flush,
        .close = File_native_close,
};

static File *FileSystem_native_open(void *_self, FilePath path, String mode)
{
    FileSystem *self = _self;
    Allocator *allocator = self->allocator;
    Buffer buf = Buffer_new(allocator);
    FilePath_to_native(self->root, &buf);
    String slash = STR("/");
    Vector_push(&buf, *Slice_at(&slash.bytes, 0));
    native_char_t *p = String_cstr(FilePath_to_native(path, &buf), allocator);
    Buffer_delete(&buf);
    native_char_t *m = String_cstr(mode, allocator);
    libsystem_FILE *file = libsystem_fopen(p, m);
    assert(file && "File exists");
    free(p);
    free(m);
    return File_new(File_native, file, self, allocator);
}

FileSystem_class FileSystem_native = {
        .open = FileSystem_native_open,
};
