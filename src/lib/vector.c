#include "../system.h"
#include "vector.h"

instantiate_vec_t(void);

void (vec_push)(void *self, const void *data, size_t size) {
    vec_t(void) *vec = (vec_t(void) *) self;
    // todo: smarter growth strategy
    vec->data = realloc(vec->data, (vec->size + 1) * size);
    memcpy((uint8_t *) vec->data + (vec->size++ * size), data, size);
}

void vec_pop(void *self) {
    vec_t(void) *vec = (vec_t(void) *) self;
    --vec->size;
    if (vec->size == 0) {
        free(vec->data);
        vec->data = NULL;
    }
}

void vec_free(void *self) {
    vec_t(void) *vec = (vec_t(void) *) self;
    vec->size = 1;
    vec_pop(vec);
}
