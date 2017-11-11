#include "_.h"

#include "../phases/eval.h"

#include <assert.h>

INTRINSIC(do, ((type_id[]) {
        ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_body = &argv[0];
    assert(arg_body->type.value == ctx->state.types.t_expr.value);

    const node_t *body = &ctx->flatten.out.data[arg_body->u.integral.value];
    return eval_list_block(ctx, body);
}
