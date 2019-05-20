#include <system.h>
#include "output.h"

compile_file compile_file_new(Allocator *allocator, InterpreterFileRef file, String ext, size_t flags)
{
    Buffer *content = malloc(sizeof(*content));
    *content = Buffer_new(allocator);
    return (compile_file) {
            .allocator = allocator,
            .file = file,
            .content = content,
            .out = Buffer_asFile(allocator, content),
            .ext = ext,
            .flags = flags,
    };
}

void compile_file_delete(compile_file *self)
{
    Allocator *allocator = self->allocator;
    fs_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}
