#include <system.h>
#include "_.h"

void compile_file_delete(compile_file *self)
{
    fs_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}
