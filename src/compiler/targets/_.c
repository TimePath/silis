#include <system.h>
#include "_.h"

compile_file compile_file_new(compilation_file_ref file)
{
    Buffer *content = malloc(sizeof(*content));
    *content = Buffer_new();
    return (compile_file) {
            .file = file,
            .content = content,
            .out = Buffer_asFile(content),
    };
}

void compile_file_delete(compile_file *self)
{
    fs_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}
