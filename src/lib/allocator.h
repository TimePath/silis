#pragma once

typedef struct {
    void *(*alloc)(void *self, size_t size);
    void *(*realloc)(void *self, void *ptr, size_t size);
    void (*free)(void *self, void *ptr);
} Allocator;

void *Allocator_alloc(Allocator *self, size_t size);

void *Allocator_realloc(Allocator *self, void *ptr, size_t size);

void Allocator_free(Allocator *self, void *ptr);
