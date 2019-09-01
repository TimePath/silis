#include <prelude.h>
#include "allocator.h"

#include "macro.h"

void *Allocator_alloc(Allocator *self, size_t size)
{
    return self->_alloc(self, size);
}

void *Allocator_realloc(Allocator *self, void *ptr, size_t size)
{
    return self->_realloc(self, ptr, size);
}

void Allocator_free(Allocator *self, void *ptr)
{
    self->_free(self, ptr);
}
