#include <system.h>
#include "vector.h"

Vector_$(void);

void (Vector_push)(void *_self, const void *data, size_t size)
{
    Vector(void) *self = (Vector(void) *) _self;
    // todo: smarter growth strategy
    Vector_data(self) = realloc(Vector_data(self), (Vector_size(self) + 1) * size);
    memcpy((uint8_t *) Vector_data(self) + (Vector_size(self)++ * size), data, size);
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
