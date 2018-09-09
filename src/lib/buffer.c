#include <system.h>
#include "buffer.h"

FILE *Buffer_asFile(Buffer *self) {
    return open_memstream((native_char_t **) &Vector_data(self), &Vector_size(self));
}
