#include <system.h>
#include "output.h"

compile_file compile_file_new(InterpreterFileRef file, String ext, size_t flags, Allocator *allocator)
{
    Buffer *content = malloc(sizeof(*content));
    *content = Buffer_new(allocator);
    return (compile_file) {
            .allocator = allocator,
            .file = file,
            .content = content,
            .out = Buffer_asFile(content, allocator),
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
