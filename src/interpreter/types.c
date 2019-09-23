#include <prelude.h>
#include "types.h"

#include <lib/misc.h>

Types Types_new(Allocator *allocator)
{
    Types _self = {
            .all = Vector_new(allocator),
            .t_untyped = Ref_null,
            .t_unit = Ref_null,
            .t_type = Ref_null,
            .t_expr = Ref_null,
            .t_string = Ref_null,
            .t_int = Ref_null,
    };
    Types *self = &_self;
    self->t_untyped = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = 0,
    });
    assert(Ref_value(self->t_untyped) == 1 && "untyped has type id 1");
    self->t_unit = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = 0,
    });
    self->t_expr = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = 0,
    });
    self->t_type = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = sizeof(TypeRef),
    });
    self->t_string = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = sizeof(const native_char_t *),
    });
    self->t_int = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque.size = sizeof(size_t),
    });
    return _self;
}

bool Types_assign(Types *self, TypeRef src, TypeRef dst)
{
    if (Ref_eq(dst, src)) {
        return true;
    }
    if (Ref_eq(dst, self->t_untyped) || Ref_eq(src, self->t_untyped)) {
        return true;
    }
    return false;
}

TypeRef Types_register(Types *self, Type it)
{
    const size_t idx = Vector_size(&self->all);
    Vector_push(&self->all, it);
    return (TypeRef) Ref_fromIndex(idx);
}

TypeRef Types_register_func(Types *self, Slice(TypeRef) types)
{
    size_t n = Slice_size(&types);
    size_t i = n - 1;
    TypeRef ret = *Slice_at(&types, i);
    while (i-- > 0) {
        TypeRef in = *Slice_at(&types, i);
        ret = Types_register(self, (Type) {
                .kind.val = Type_Function,
                .u.Function = { .in = in, .out = ret },
        });
    }
    return ret;
}

const Type *Types_lookup(const Types *self, TypeRef ref)
{
    return Vector_at(&self->all, Ref_toIndex(ref));
}

TypeRef Types_function_result(const Types *self, TypeRef T)
{
    TypeRef ret = Ref_null;
    for (const Type *it = Types_lookup(self, T); it->kind.val == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ret = it->u.Function.out;
    }
    return ret;
}

size_t Types_function_arity(const Types *self, TypeRef T)
{
    size_t argc = 0;
    for (const Type *it = Types_lookup(self, T); it->kind.val == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ++argc;
    }
    return argc;
}
