#include <prelude.h>
#include "output.h"

#include <lib/fs/memoryfile.h>
#include <lib/misc.h>

compile_file compile_file_new(Ref(InterpreterFile) file, String ext, size_t stage, Slice(file_flag) flags, Allocator *allocator)
{
    Buffer *content = new(Buffer, Buffer_new(allocator));
    size_t flagsMask = 0;
    Slice_loop(&flags, i) {
        flagsMask |= 1 << *Slice_at(&flags, i);
    }
    return (compile_file) {
            .allocator = allocator,
            .file = file,
            .content = content,
            .out = MemoryFile_new(content),
            .ext = ext,
            .stage = stage,
            .flags = flagsMask,
    };
}

void compile_file_delete(compile_file *self)
{
    Allocator *allocator = self->allocator;
    File_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}
