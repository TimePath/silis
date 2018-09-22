#pragma once

#include <lib/vector.h>

#include "type.h"

typedef struct types_s {
    Vector(type_t) all;
    /// can be produced with `()`
    type_id t_unit;
    /// typedefs have this type
    type_id t_type;
    /// unevaluated code has this type
    /// when calling such a function, quasiquote the input
    /// todo: more specific types
    /// expr<void> ; returns void
    /// expr<T> ; returns T, whatever that is
    type_id t_expr;
    /// strings
    type_id t_string;
    /// integers
    type_id t_int;
    /// index of last intrinsic type
    size_t end_intrinsics;
} types_t;

types_t types_new(void);

type_id type_new(types_t *ctx, type_t it);

type_id type_func_new(types_t *ctx, type_id *argv, size_t n);

type_id type_func_ret(const types_t *ctx, const type_t *T);

size_t type_func_argc(const types_t *ctx, const type_t *T);

const type_t *type_lookup(const types_t *ctx, type_id id);