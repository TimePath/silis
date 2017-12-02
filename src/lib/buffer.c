#include "../system.h"
#include "buffer.h"

FILE *buf_file(buffer_t *self) {
    return open_memstream(&self->data, &self->size);
}
