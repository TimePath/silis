#include <system.h>
#include "types.h"

types_t types_new(Allocator *allocator)
{
    types_t _self = {
            .all = Vector_new(allocator),
            .t_untyped = {.value = 0},
            .t_unit = {.value = 0},
            .t_type = {.value = 0},
            .t_expr = {.value = 0},
            .t_string = {.value = 0},
            .t_int = {.value = 0},
    };
    types_t *self = &_self;
    self->t_untyped = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    assert(self->t_untyped.value == 1 && "untyped has type id 1");
    self->t_unit = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    self->t_expr = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    self->t_type = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(type_id),
    });
    self->t_string = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(const native_char_t *),
    });
    self->t_int = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(size_t),
    });
    return _self;
}

type_id type_new(types_t *ctx, type_t it)
{
    Vector_push(&ctx->all, it);
    return (type_id) {.value = Vector_size(&ctx->all)};
}

bool type_assignable_to(types_t *ctx, type_id self, type_id other)
{
    if (self.value == other.value) {
        return true;
    }
    if (self.value == ctx->t_untyped.value) {
        return true;
    }
    return false;
}

type_id type_func_new(types_t *ctx, type_id *argv, size_t n)
{
    size_t i = n - 1;
    type_id ret = argv[i];
    while (i-- > 0) {
        type_id in = argv[i];
        ret = type_new(ctx, (type_t) {
                .kind = TYPE_FUNCTION,
                .u.func.in = in,
                .u.func.out = ret,
        });
    }
    return ret;
}

type_id type_func_ret(const types_t *ctx, const type_t *T)
{
    type_id ret = {.value = 0};
    for (const type_t *it = T; it->kind == TYPE_FUNCTION; it = type_lookup(ctx, it->u.func.out)) {
        ret = it->u.func.out;
    }
    return ret;
}

size_t type_func_argc(const types_t *ctx, const type_t *T)
{
    size_t argc = 0;
    for (const type_t *it = T; it->kind == TYPE_FUNCTION; it = type_lookup(ctx, it->u.func.out)) {
        ++argc;
    }
    return argc;
}

const type_t *type_lookup(const types_t *ctx, type_id id)
{
    return Vector_at(&ctx->all, id.value - 1);
}
