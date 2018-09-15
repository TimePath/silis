#include <system.h>
#include "vector.h"

Vector_$(void);

void (Vector_push)(size_t sizeof_T, void *_self, size_t dataSize, const void *data)
{
    Vector(void) *self = (Vector(void) *) _self;
    const size_t n = Vector_size(self);
    // todo: smarter growth strategy
    Vector_data(self) = realloc(Vector_data(self), (n + 1) * sizeof_T);
    memcpy((uint8_t *) Vector_data(self) + (n * sizeof_T), data, dataSize);
    ++Vector_size(self);
}

void Vector_pop(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    --Vector_size(self);
    if (Vector_size(self) == 0) {
        free(Vector_data(self));
        Vector_data(self) = NULL;
    }
}

void Vector_delete(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Vector_size(self) = 0;
    free(Vector_data(self));
    Vector_data(self) = NULL;
}
