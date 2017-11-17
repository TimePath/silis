#include "_.h"

#include "../phases/eval.h"

#include <assert.h>

INTRINSIC(define, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_name = &argv[0];
    assert(arg_name->type.value == ctx->state.types.t_expr.value);
    const value_t *arg_val = &argv[1];
    assert(arg_val->type.value == ctx->state.types.t_expr.value);

    const node_t *name = &ctx->flatten.out.data[arg_name->u.integral.value];
    assert(name->type == NODE_ATOM);
    const node_t *val = &ctx->flatten.out.data[arg_val->u.integral.value];

    const value_t v = eval_node(ctx, val);
    sym_def(ctx, name->u.atom.value, (sym_t) {
            .type = v.type,
            .value = v,
    });
    return (value_t) {.type = ctx->state.types.t_unit};
}
