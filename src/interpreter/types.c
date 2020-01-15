#include <prelude.h>
#include "types.h"

#include <lib/misc.h>

void Types_new(Types *self, Allocator *allocator)
{
    *self = (Types) {
            .all = Vector_new(Type, allocator),
            .t_untyped = Ref_null,
            .t_unit = Ref_null,
            .t_type = Ref_null,
            .t_expr = Ref_null,
            .t_string = Ref_null,
            .t_int = Ref_null,
    };
    self->t_untyped = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = 0,},
    });
    assert(Ref_value(self->t_untyped) == 1 && "untyped has type id 1");
    self->t_unit = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = 0,},
    });
    self->t_expr = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = 0,},
    });
    self->t_type = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = sizeof(Ref(Type)),},
    });
    self->t_string = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = sizeof(const native_char_t *),},
    });
    self->t_int = Types_register(self, (Type) {
            .kind.val = Type_Opaque,
            .u.Opaque = {.size = sizeof(size_t),},
    });
}

bool Types_assign(Types *self, Ref(Type) src, Ref(Type) dst)
{
    if (Ref_eq(dst, src)) {
        return true;
    }
    if (Ref_eq(dst, self->t_untyped) || Ref_eq(src, self->t_untyped)) {
        return true;
    }
    return false;
}

Ref(Type) Types_register(Types *self, Type it)
{
    const size_t idx = Vector_size(&self->all);
    Vector_push(&self->all, it);
    return (Ref(Type)) Vector_ref(&self->all, idx);
}

Ref(Type) Types_register_func(Types *self, Slice(Ref(Type)) types)
{
    size_t n = Slice_size(&types);
    size_t i = n - 1;
    Ref(Type) ret = *Slice_at(&types, i);
    while (i-- > 0) {
        Ref(Type) in = *Slice_at(&types, i);
        ret = Types_register(self, (Type) {
                .kind.val = Type_Function,
                .u.Function = { .in = in, .out = ret },
        });
    }
    return ret;
}

const Type *Types_lookup(const Types *self, Ref(Type) ref)
{
    assert(Ref_toBool(ref));
    return Vector_at(&self->all, Ref_toIndex(ref));
}

Ref(Type) Types_function_result(const Types *self, Ref(Type) T)
{
    Ref(Type) ret = Ref_null;
    for (const Type *it = Types_lookup(self, T); it->kind.val == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ret = it->u.Function.out;
    }
    return ret;
}

size_t Types_function_arity(const Types *self, Ref(Type) T)
{
    size_t argc = 0;
    for (const Type *it = Types_lookup(self, T); it->kind.val == Type_Function; it = Types_lookup(self, it->u.Function.out)) {
        ++argc;
    }
    return argc;
}
