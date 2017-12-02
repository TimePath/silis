#include "../system.h"

#include "_.h"

#include "../phases/eval.h"

INTRINSIC(do, ((type_id[]) {
        ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_body = &argv[0];

    const node_t *body = node_get(ctx, arg_body->u.expr.value);
    return eval_list_block(ctx, body);
}
