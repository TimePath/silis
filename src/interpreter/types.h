#pragma once

#include <lib/vector.h>

#include "type.h"

typedef struct Types {
    Vector(Type) all;
    /// result of untyped, assignable to anything
    TypeRef t_untyped;
    /// can be produced with `()`
    TypeRef t_unit;
    /// typedefs have this type
    TypeRef t_type;
    /// unevaluated code has this type
    /// when calling such a function, quasiquote the input
    /// todo: more specific types
    /// expr<void> ; returns void
    /// expr<T> ; returns T, whatever that is
    TypeRef t_expr;
    /// strings
    TypeRef t_string;
    /// integers
    TypeRef t_int;
} Types;

Types Types_new(Allocator *allocator);

bool Types_assign(Types *self, TypeRef src, TypeRef dst);

TypeRef Types_register(Types *self, Type it);

TypeRef Types_register_func(Types *self, Slice(TypeRef) types);

const Type *Types_lookup(const Types *self, TypeRef ref);

TypeRef Types_function_result(const Types *self, TypeRef T);

size_t Types_function_arity(const Types *self, TypeRef T);
