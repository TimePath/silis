#include <prelude.h>
#include "vector.h"

#include <system.h>

Vector_instantiate(void);

void _Vector_push(size_t sizeof_T, void *_self, size_t dataSize, const void *data, size_t count)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    size_t oldSize = Vector_size(self);
    // todo: smarter growth strategy
    size_t size = oldSize + count;
    self->_data = realloc(self->_data, sizeof_T * size);
    self->_size = size;
    libsystem_memcpy(_Vector_at(uint8_t, sizeof_T, self, oldSize), data, dataSize);
}

void Vector_pop(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    self->_size -= 1;
    if (Vector_size(self) == 0) {
        free(self->_data);
        self->_data = NULL;
    }
}

void _Vector_delete(void *_self)
{
    Vector(void) *self = (Vector(void) *) _self;
    Allocator *allocator = self->_allocator;
    self->_size = 0;
    free(self->_data);
    self->_data = NULL;
}
