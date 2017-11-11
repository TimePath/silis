#include "_.h"

#include "../phases/eval.h"

#include <assert.h>

INTRINSIC(cond, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_predicate = &argv[0];
    assert(arg_predicate->type.value == ctx->state.types.t_expr.value);
    const value_t *arg_true = &argv[1];
    assert(arg_true->type.value == ctx->state.types.t_expr.value);
    const value_t *arg_false = &argv[2];
    assert(arg_false->type.value == ctx->state.types.t_expr.value);

    const node_t *predicate = &ctx->flatten.out.data[arg_predicate->u.integral.value];
    const node_t *node_true = &ctx->flatten.out.data[arg_true->u.integral.value];
    const node_t *node_false = &ctx->flatten.out.data[arg_false->u.integral.value];
    const value_t ret = eval_node(ctx, predicate);
    return eval_node(ctx, ret.u.integral.value != 0 ? node_true : node_false);
}
