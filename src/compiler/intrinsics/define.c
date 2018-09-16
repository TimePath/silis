#include <system.h>
#include "define.h"

#include "_.h"
#include "../phases/eval.h"

INTRINSIC_IMPL(define, "define", ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
}))
{
    const value_t *arg_name = &argv[0];
    const value_t *arg_val = &argv[1];

    const node_t *name = node_get(ctx, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    const node_t *val = node_get(ctx, arg_val->u.expr.value);

    const value_t v = eval_node(ctx, val);
    sym_def(ctx, name->u.atom.value, (sym_t) {
            .type = v.type,
            .value = v,
    });
    return (value_t) {.type = ctx->state.types.t_unit};
}
