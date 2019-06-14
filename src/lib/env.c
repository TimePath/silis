#include <system.h>
#include "env.h"

#include "allocator.h"
#include "string.h"
#include "vector.h"

static void *CAllocator_alloc(void *self, size_t size);

static void *CAllocator_realloc(void *self, void *ptr, size_t size);

static void CAllocator_free(void *self, void *ptr);

typedef struct {
    Allocator interface;
    Allocator *implementation;
    size_t size;
    size_t max_size;
} DebugAllocator;

typedef struct {
    size_t size;
} DebugAllocation;

static void *DebugAllocator_alloc(void *_self, size_t size);

static void *DebugAllocator_realloc(void *_self, void *ptr, size_t size);

static void DebugAllocator_free(void *_self, void *ptr);

size_t Env_run(size_t argc, native_string_t argv[], size_t (*run)(Env)) {
    extern size_t strlen(native_string_t __s);

    Allocator _allocator = (Allocator) {
            ._alloc = CAllocator_alloc,
            ._realloc = CAllocator_realloc,
            ._free = CAllocator_free,
    };
    Allocator *cAllocator = &_allocator;

    DebugAllocator debugAllocator = (DebugAllocator) {
            .interface = {._alloc = DebugAllocator_alloc, ._realloc = DebugAllocator_realloc, ._free = DebugAllocator_free},
            .implementation = cAllocator,
    };
    Allocator *allocator = &debugAllocator.interface;
    extern File_class File_native;
    File *out = File_new(File_native, libsystem_stdout(), NULL, allocator);
    extern FileSystem_class FileSystem_native;
    FileSystem fs = FileSystem_new(FileSystem_native, &fs, allocator);
    Vector(String) args = Vector_new(allocator);
    for (size_t i = 0; i < (size_t) argc; ++i) {
        native_string_t cstr = argv[i];
        Slice(uint8_t) slice = {._begin.r = (const uint8_t *) cstr, ._end = (const uint8_t *) (cstr + strlen(cstr))};
        String s = String_fromSlice(slice, ENCODING_SYSTEM);
        Vector_push(&args, s);
    }
    size_t ret = run((Env) {
            .args = Vector_toSlice(String, &args),
            .out = out,
            .fs = &fs,
            .allocator = allocator,
    });
    Vector_delete(String, &args);
    FileSystem_delete(&fs);
    File_close(out);

    assert(!debugAllocator.size && "no memory leaked");

    return ret;
}

static void *CAllocator_alloc(void *self, size_t size) {
    (void) self;
    extern void *(malloc)(size_t size);
    return (malloc)(size);
}

static void *CAllocator_realloc(void *self, void *ptr, size_t size) {
    (void) self;
    extern void *(realloc)(void *ptr, size_t size);
    return (realloc)(ptr, size);
}

static void CAllocator_free(void *self, void *ptr) {
    (void) self;
    extern void (free)(void *ptr);
    (free)(ptr);
}

static void *DebugAllocator_alloc(void *_self, size_t size) {
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = Allocator_alloc(self->implementation, sizeof(DebugAllocation) + size);
    mem->size = size;
    self->size += size;
    self->max_size = self->size > self->max_size ? self->size : self->max_size;
    return mem + 1;
}

static void *DebugAllocator_realloc(void *_self, void *ptr, size_t size) {
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = ptr ? ((DebugAllocation *) ptr) - 1 : NULL;
    if (mem) {
        self->size -= mem->size;
    }
    mem = Allocator_realloc(self->implementation, mem, sizeof(DebugAllocation) + size);
    mem->size = size;
    self->size += size;
    self->max_size = self->size > self->max_size ? self->size : self->max_size;
    return mem + 1;
}

static void DebugAllocator_free(void *_self, void *ptr) {
    if (!ptr) return;
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = ((DebugAllocation *) ptr) - 1;
    self->size -= mem->size;
    Allocator_free(self->implementation, mem);
}
