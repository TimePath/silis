#include <system.h>

#include "vector.h"

Vector_$(void);

void (Vector_push)(void *_self, const void *data, size_t size)
{
    Vector(void) *self = (Vector(void) *) _self;
    // todo: smarter growth strategy
    self->data = realloc(self->data, (self->size + 1) * size);
    memcpy((uint8_t *) self->data + (self->size++ * size), data, size);
}

void Vector_pop(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    --self->size;
    if (self->size == 0) {
        free(self->data);
        self->data = NULL;
    }
}

void Vector_delete(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    self->size = 0;
    free(self->data);
    self->data = NULL;
}
