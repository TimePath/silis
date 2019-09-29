#pragma once

#include <lib/vector.h>

#include "type.h"

typedef struct Types {
    Vector(Type) all;
    /// result of untyped, assignable to anything
    Ref(Type) t_untyped;
    /// can be produced with `()`
    Ref(Type) t_unit;
    /// typedefs have this type
    Ref(Type) t_type;
    /// unevaluated code has this type
    /// when calling such a function, quasiquote the input
    /// todo: more specific types
    /// expr<void> ; returns void
    /// expr<T> ; returns T, whatever that is
    Ref(Type) t_expr;
    /// strings
    Ref(Type) t_string;
    /// integers
    Ref(Type) t_int;
} Types;

void Types_new(Types *self, Allocator *allocator);

bool Types_assign(Types *self, Ref(Type) src, Ref(Type) dst);

Ref(Type) Types_register(Types *self, Type it);

Ref(Type) Types_register_func(Types *self, Slice(Ref(Type)) types);

const Type *Types_lookup(const Types *self, Ref(Type) ref);

Ref(Type) Types_function_result(const Types *self, Ref(Type) T);

size_t Types_function_arity(const Types *self, Ref(Type) T);
