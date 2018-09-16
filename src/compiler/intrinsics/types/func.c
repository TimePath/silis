#include <system.h>
#include "func.h"

#include "../_.h"
#include "../../phases/eval.h"

static void types_func_args_types(ctx_t *ctx, const node_t *args, size_t argc, type_id *out);

INTRINSIC_IMPL(types_func, "types/func", ((type_id[]) {
        ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_args = &argv[0];

    const node_t *args = node_get(ctx, arg_args->u.expr.value);
    assert(args->kind == NODE_LIST_BEGIN);
    const size_t argc = args->u.list.size;
    assert(argc >= 2 && "has enough arguments");

    type_id Ts[argc];
    types_func_args_types(ctx, node_list_children(args), argc, Ts);

    return (value_t) {
            .type = ctx->state.types.t_type,
            .u.type.value = type_func_new(ctx, Ts, argc),
    };
}

static void types_func_args_types(ctx_t *ctx, const node_t *args, size_t argc, type_id *out) {
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(ctx, &args[i]);
        const value_t type = eval_node(ctx, it);
        if (type.type.value == ctx->state.types.t_unit.value) {
            out[i] = ctx->state.types.t_unit;
            continue;
        }
        assert(type.type.value == ctx->state.types.t_type.value);
        out[i] = type.u.type.value;
    }
}
