#include "func.h"

#include "_.h"
#include "../phases/eval.h"

#include <assert.h>

static void func_args_types(ctx_t *ctx, const node_t *args, size_t argc, type_id out[argc]);

INTRINSIC(func, ((type_id[]) {
        ctx->state.types.t_expr, ctx->state.types.t_expr,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_args = &argv[0];
    assert(arg_args->type.value == ctx->state.types.t_expr.value);
    const value_t *arg_body = &argv[1];
    assert(arg_body->type.value == ctx->state.types.t_expr.value);

    const node_t *args = &ctx->flatten.out.data[arg_args->u.integral.value];
    assert(args->type == NODE_LIST_BEGIN);
    const size_t argc = args->u.list.size;
    assert(argc >= 2 && "not enough arguments");

    type_id Ts[argc];
    func_args_types(ctx, args + 1, argc, Ts);

    return (value_t) {
            .type = type_func_new(ctx, Ts, argc),
            .u.func.value = arg_body->u.integral.value,
            .u.func.arglist = arg_args->u.integral.value,
    };
}

static void func_args_types(ctx_t *ctx, const node_t *args, size_t argc, type_id out[argc]) {
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = eval_deref(ctx, &args[i]);
        assert(it->type == NODE_LIST_BEGIN);
        const size_t n = it->u.list.size;
        if (n == 2) {
            const node_t *children = it + 1;
            const node_t *node_type = &children[0];
            const value_t type = eval_node(ctx, node_type);
            assert(type.type.value == ctx->state.types.t_type.value);
            out[i] = (type_id) {.value = type.u.integral.value};
            const node_t *node_id = &children[1];
            assert(node_id->type == NODE_ATOM);
        } else if (n == 0) {
            out[i] = ctx->state.types.t_unit;
        } else {
            assert(false);
        }
    }
}

static void func_args_load(ctx_t *ctx, const node_t *arglist, const value_t *argv);

value_t func_call(ctx_t *ctx, value_t func, const value_t *argv) {
    const node_t *body = &ctx->flatten.out.data[func.u.func.value];
    const node_t *arglist = &ctx->flatten.out.data[func.u.func.arglist];
    func_args_load(ctx, arglist, argv);
    return eval_node(ctx, body);
}

static void func_args_load(ctx_t *ctx, const node_t *arglist, const value_t *argv) {
    assert(arglist->type == NODE_LIST_BEGIN);
    const size_t argc = arglist->u.list.size;
    const node_t *args = arglist + 1;
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = eval_deref(ctx, &args[i]);
        assert(it->type == NODE_LIST_BEGIN);
        const size_t n = it->u.list.size;
        if (n == 2) {
            const node_t *children = it + 1;
            const node_t *node_id = &children[1];
            assert(node_id->type == NODE_ATOM);

            const value_t *v = &argv[i];
            sym_t sym = (sym_t) {
                    .name = node_id->u.atom.value,
                    .type = v->type,
                    .value = *v,
            };
            vec_push(&ctx->state.symbols, sym);
        }
    }
}
