#include <system.h>
#include "vector.h"

Vector_instantiate(void);

void _Vector_push(size_t sizeof_T, void *_self, size_t dataSize, const void *data, size_t count)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    const size_t size = Vector_size(self);
    // todo: smarter growth strategy
    _Vector_data(self) = realloc(_Vector_data(self), (size + count) * sizeof_T);
    memcpy((uint8_t *) _Vector_data(self) + (size * sizeof_T), data, dataSize);
    Vector_size(self) += count;
}

void Vector_pop(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    --Vector_size(self);
    if (Vector_size(self) == 0) {
        free(_Vector_data(self));
        _Vector_data(self) = NULL;
    }
}

void _Vector_delete(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    Vector_size(self) = 0;
    free(_Vector_data(self));
    _Vector_data(self) = NULL;
}
