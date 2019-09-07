#include <prelude.h>
#include "native.h"

#include <system.h>

#include "../fs.h"
#include "../misc.h"
#include "../slice.h"

extern File_class File_native;

extern FileSystem_class FileSystem_native;

static size_t File_native_read(void *self, Slice(uint8_t) out)
{
    return libsystem_fread(Slice_data_mut(&out), sizeof(uint8_t), Slice_size(&out), self);
}

static size_t File_native_write(void *self, Slice(uint8_t) in)
{
    return libsystem_fwrite(_Slice_data(&in), sizeof(uint8_t), Slice_size(&in), self);
}

static size_t File_native_size(void *self)
{
    libsystem_fseek(self, 0, libsystem_SEEK_END);
    native_long_t ret = libsystem_ftell(self);
    libsystem_fseek(self, 0, libsystem_SEEK_SET);
    if (ret < 0) {
        return 0;
    }
    return (size_t) ret;
}

static bool File_native_rewind(void *self)
{
    return libsystem_fseek(self, 0, libsystem_SEEK_SET) == 0;;
}

static bool File_native_flush(void *self)
{
    return libsystem_fflush(self) == 0;
}

static bool File_native_close(void *self)
{
    return libsystem_fclose(self) == 0;
}

File_class File_native = {
        .read = File_native_read,
        .write = File_native_write,
        .size = File_native_size,
        .rewind = File_native_rewind,
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
