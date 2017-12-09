#include "../system.h"
#include "buffer.h"

FILE *buf_file(buffer_t *self) {
    return open_memstream((native_char_t **) self->data, &self->size);
}
