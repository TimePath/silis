#include <prelude.h>
#include "output.h"

#include <lib/fs/memoryfile.h>
#include <lib/misc.h>

compile_file compile_file_new(InterpreterFileRef file, String ext, size_t flags, Allocator *allocator)
{
    Buffer *content = new(Buffer, Buffer_new(allocator));
    return (compile_file) {
            .allocator = allocator,
            .file = file,
            .content = content,
            .out = MemoryFile_new(content),
            .ext = ext,
            .flags = flags,
    };
}

void compile_file_delete(compile_file *self)
{
    Allocator *allocator = self->allocator;
    File_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}
