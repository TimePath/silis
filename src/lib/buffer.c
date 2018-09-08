#include <system.h>

#include "buffer.h"

FILE *Buffer_toFile(Buffer *self) {
    return open_memstream((native_char_t **) &self->data, &self->size);
}
