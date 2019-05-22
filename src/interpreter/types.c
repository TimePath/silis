#include <system.h>
#include "types.h"

Types Types_new(Allocator *allocator)
{
    Types _self = {
            .all = Vector_new(allocator),
            .t_untyped = {.value = 0},
            .t_unit = {.value = 0},
            .t_type = {.value = 0},
            .t_expr = {.value = 0},
            .t_string = {.value = 0},
            .t_int = {.value = 0},
    };
    Types *self = &_self;
    self->t_untyped = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = 0,
    });
    assert(self->t_untyped.value == 1 && "untyped has type id 1");
    self->t_unit = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = 0,
    });
    self->t_expr = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = 0,
    });
    self->t_type = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = sizeof(TypeRef),
    });
    self->t_string = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = sizeof(const native_char_t *),
    });
    self->t_int = Types_register(self, (Type) {
            .kind = Type_Opaque,
            .u.Opaque.size = sizeof(size_t),
    });
    return _self;
}

bool Types_assign(Types *self, TypeRef src, TypeRef dst)
{
    if (src.value == dst.value) {
        return true;
    }
    if (dst.value == self->t_untyped.value) {
        return true;
    }
    if (src.value == self->t_untyped.value) {
        return true;
    }
    return false;
}

TypeRef Types_register(Types *self, Type it)
{
    Vector_push(&self->all, it);
    return (TypeRef) {.value = Vector_size(&self->all)};
}

TypeRef Types_register_func(Types *self, Slice(TypeRef) types)
{
    size_t n = Slice_size(&types);
    size_t i = n - 1;
    TypeRef ret = *Slice_at(&types, i);
    while (i-- > 0) {
        TypeRef in = *Slice_at(&types, i);
        ret = Types_register(self, (Type) {
                .kind = Type_Function,
                .u.Function = { .in = in, .out = ret },
        });
    }
    return ret;
}

const Type *Types_lookup(const Types *self, TypeRef ref)
{
    return Vector_at(&self->all, ref.value - 1);
}

TypeRef Types_function_result(const Types *self, TypeRef T)
{
    TypeRef ret = {.value = 0};
    for (const Type *it = Types_lookup(self, T); it->kind == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ret = it->u.Function.out;
    }
    return ret;
}

size_t Types_function_arity(const Types *self, TypeRef T)
{
    size_t argc = 0;
    for (const Type *it = Types_lookup(self, T); it->kind == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ++argc;
    }
    return argc;
}
