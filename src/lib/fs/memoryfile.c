#include <prelude.h>
#include "memoryfile.h"

#include "../fs.h"
#include "../slice.h"

static size_t File_memory_write(void *self, Slice(uint8_t) in);

static File_class File_memory = {
        .write = File_memory_write,
};

File *MemoryFile_new(Buffer *buf)
{
    return File_new(File_memory, buf, NULL, buf->_allocator);
}

static size_t File_memory_write(void *_self, Slice(uint8_t) in)
{
    Buffer *self = _self;
    size_t n = Slice_size(&in);
    const uint8_t *d = _Slice_data(&in);
    _Vector_push(sizeof(uint8_t), self, n, d, n);
    return n;
}
