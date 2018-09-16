#include <system.h>
#include "cond.h"

#include "_.h"
#include "../phases/eval.h"

INTRINSIC_IMPL(cond, "cond", ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
}))
{
    const value_t *arg_predicate = &argv[0];
    const value_t *arg_true = &argv[1];
    const value_t *arg_false = &argv[2];

    const node_t *predicate = node_get(ctx, arg_predicate->u.expr.value);
    const node_t *node_true = node_get(ctx, arg_true->u.expr.value);
    const node_t *node_false = node_get(ctx, arg_false->u.expr.value);
    const value_t ret = eval_node(ctx, predicate);
    return eval_node(ctx, ret.u.integral.value != 0 ? node_true : node_false);
}
