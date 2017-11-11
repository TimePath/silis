#include "_.h"

#include <assert.h>

INTRINSIC(defun, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_name = &argv[0];
    assert(arg_name->type.value == ctx->state.types.t_expr.value);
    // const value_t *args = &argv[1]; // todo: use
    const value_t *arg_body = &argv[2];
    assert(arg_body->type.value == ctx->state.types.t_expr.value);

    const node_t *name = &ctx->flatten.out.data[arg_name->u.integral.value];

    sym_t sym = (sym_t) {
            .name = name->text,
            .type = type_func_new(ctx, (type_id[]) {
                    ctx->state.types.t_unit,
                    ctx->state.types.t_unit,
            }, 2),
            .value = *arg_body,
    };
    vec_push(&ctx->state.symbols, sym);
    return (value_t) {.type = ctx->state.types.t_unit};
}
