#include <system.h>
#include "allocator.h"

#include "macro.h"

void *Allocator_alloc(Allocator *self, size_t size)
{
    return self->alloc(self, size);
}

void *Allocator_realloc(Allocator *self, void *ptr, size_t size)
{
    return self->NOEXPAND(realloc)(self, ptr, size);
}

void Allocator_free(Allocator *self, void *ptr)
{
    self->NOEXPAND(free)(self, ptr);
}
