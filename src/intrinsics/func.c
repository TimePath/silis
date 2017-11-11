#include "_.h"

#include <assert.h>

INTRINSIC(func, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    // const value_t *args = &argv[0]; // todo: use
    const value_t *arg_body = &argv[1];
    assert(arg_body->type.value == ctx->state.types.t_expr.value);

    value_t ret = *arg_body;
    ret.type = type_func_new(ctx, (type_id[]) {
            ctx->state.types.t_unit,
            ctx->state.types.t_unit,
    }, 2);
    return ret;
}
