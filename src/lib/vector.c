#include <system.h>
#include "vector.h"

Vector_instantiate(void);

void _Vector_push(size_t sizeof_T, void *_self, size_t dataSize, const void *data, size_t n)
{
    Vector(void) *self = (Vector(void) *) _self;
    const size_t size = Vector_size(self);
    // todo: smarter growth strategy
    Vector_data(self) = realloc(Vector_data(self), (size + n) * sizeof_T);
    memcpy((uint8_t *) Vector_data(self) + (size * sizeof_T), data, dataSize);
    Vector_size(self) += n;
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

void _Vector_delete(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Vector_size(self) = 0;
    free(Vector_data(self));
    Vector_data(self) = NULL;
}
