#include <system.h>
#include "buffer.h"

#include "fs.h"

static ssize_t File_memory_write(void *self, Slice(uint8_t) in);

static File_class File_memory = {
        .write = File_memory_write,
};

File *Buffer_asFile(Allocator *allocator, Buffer *self)
{
    return fs_open_(allocator, File_memory, self);
}

static ssize_t File_memory_write(void *_self, Slice(uint8_t) in)
{
    Buffer *self = _self;
    size_t n = Slice_size(&in);
    const uint8_t *d = _Slice_data(&in);
    _Vector_push(sizeof(uint8_t), self, n, d, n);
    return (ssize_t) n;
}
