#include <system.h>

#include "_.h"
#include "../phases/eval.h"

INTRINSIC(extern, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_name = &argv[0];
    const value_t *arg_val = &argv[1];

    const node_t *name = node_get(ctx, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    const node_t *val = node_get(ctx, arg_val->u.expr.value);

    const value_t v = eval_node(ctx, val);
    assert(v.type.value == ctx->state.types.t_type.value);
    sym_def(ctx, name->u.atom.value, (sym_t) {
            .type = v.u.type.value,
            .flags.native = true,
    });
    return (value_t) {.type = ctx->state.types.t_unit};
}