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
    const value_t *arg_body = &argv[1];

    const node_t *args = ctx_node(ctx, arg_args->u.expr.value);
    assert(args->kind == NODE_LIST_BEGIN);
    const size_t argc = args->u.list.size;
    assert(argc >= 2 && "has enough arguments");

    type_id Ts[argc];
    func_args_types(ctx, NODE_LIST_CHILDREN(args), argc, Ts);

    return (value_t) {
            .type = type_func_new(ctx, Ts, argc),
            .u.func.value = arg_body->u.expr.value,
            .u.func.arglist = arg_args->u.expr.value,
    };
}

static void func_args_types(ctx_t *ctx, const node_t *args, size_t argc, type_id out[argc]) {
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(ctx, &args[i]);
        assert(it->kind == NODE_LIST_BEGIN);
        const size_t n = it->u.list.size;
        if (n == 0) {
            out[i] = ctx->state.types.t_unit;
        } else if (n <= 2) {
            const node_t *children = NODE_LIST_CHILDREN(it);
            const node_t *node_type = &children[0];
            const value_t type = eval_node(ctx, node_type);
            assert(type.type.value == ctx->state.types.t_type.value);
            out[i] = type.u.type.value;
            if (n == 2) {
                const node_t *node_id = &children[1];
                assert(node_id->kind == NODE_ATOM);
                (void) (node_id);
            }
        } else {
            assert(false);
        }
    }
}

void func_args_names(const ctx_t *ctx, const node_t *args, size_t argc, string_view_t out[argc]) {
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(ctx, &args[i]);
        assert(it->kind == NODE_LIST_BEGIN);
        const size_t n = it->u.list.size;
        if (n != 2) {
            out[i] = STR("");
        } else {
            const node_t *children = NODE_LIST_CHILDREN(it);
            const node_t *node_id = &children[1];
            assert(node_id->kind == NODE_ATOM);
            out[i] = node_id->u.atom.value;
        }
    }
}

static void func_args_load(ctx_t *ctx, const node_t *arglist, const value_t *argv);

value_t func_call(ctx_t *ctx, value_t func, const value_t *argv) {
    if (func.type.value <= ctx->state.types.end_intrinsics) {
        return func.u.intrinsic.value(ctx, argv);
    }
    sym_push(ctx, 0);
    const node_t *body = ctx_node(ctx, func.u.func.value);
    const node_t *arglist = ctx_node(ctx, func.u.func.arglist);
    func_args_load(ctx, arglist, argv);
    const value_t ret = eval_node(ctx, body);
    sym_pop(ctx);
    return ret;
}

static void func_args_load(ctx_t *ctx, const node_t *arglist, const value_t *argv) {
    assert(arglist->kind == NODE_LIST_BEGIN);
    const size_t argc = arglist->u.list.size;
    const node_t *args = NODE_LIST_CHILDREN(arglist);
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(ctx, &args[i]);
        assert(it->kind == NODE_LIST_BEGIN);
        const size_t n = it->u.list.size;
        if (n == 2) {
            const node_t *children = NODE_LIST_CHILDREN(it);
            const node_t *node_id = &children[1];
            assert(node_id->kind == NODE_ATOM);

            const value_t *v = &argv[i];
            sym_def(ctx, node_id->u.atom.value, (sym_t) {
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}
