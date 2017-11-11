#include "eval.h"
#include "../ctx.h"
#include "../intrinsics/func.h"

#include <assert.h>

void do_eval(ctx_t *ctx) {
    const node_t *it = &ctx->flatten.out.data[ctx->flatten.out.size - 1];
    assert(it->type == NODE_LIST_END);
    while ((--it)->type != NODE_LIST_BEGIN);
    eval_list_block(ctx, it);
}

value_t eval_list_block(ctx_t *ctx, const node_t *it) {
    assert(it->type == NODE_LIST_BEGIN);
    const size_t n = it->u.list.size;
    const node_t *children = it + 1;
    value_t ret = (value_t) {.type = ctx->state.types.t_unit};
    for (size_t i = 0; i < n; ++i) {
        ret = eval_node(ctx, &children[i]);
    }
    return ret;
}

const node_t *eval_deref(const ctx_t *ctx, const node_t *it) {
    if (it->type == NODE_REF) {
        const node_t *ret = &ctx->flatten.out.data[it->u.ref.value];
        assert(ret->type != NODE_REF && "no double refs");
        return ret;
    }
    return it;
}

value_t eval_node(ctx_t *ctx, const node_t *it) {
    it = eval_deref(ctx, it);
    if (it->type != NODE_LIST_BEGIN) {
        return val_from(ctx, it);
    }
    const size_t n = it->u.list.size;
    const node_t *children = it + 1;
    if (n == 1) {
        return eval_node(ctx, &children[0]);
    }
    if (!n) {
        return (value_t) {.type = ctx->state.types.t_unit};
    }
    const vec_t(type_t) *types = &ctx->state.types.all;
    const value_t func = eval_node(ctx, &children[0]);
    const type_t *T = &types->data[func.type.value];
    assert(T->type == TYPE_FUNCTION);

    const size_t ofs = ctx->eval.stack.size;
    const type_id expr_t = ctx->state.types.t_expr;
    size_t T_argc = 0;
    // holds return value after this loop
    const type_t *link = T;
    for (; link->type == TYPE_FUNCTION; ++T_argc) {
        assert((n - 1) > T_argc && "argument underflow");
        const node_t *arg = &children[T_argc + 1];
        const type_id arg_t = link->u.func.in;
        if (arg_t.value == expr_t.value) {
            value_t v = (value_t) {
                    .type = expr_t,
                    .u.integral.value = eval_deref(ctx, arg) - ctx->flatten.out.data,
            };
            vec_push(&ctx->eval.stack, v);
        } else {
            value_t v = eval_node(ctx, arg);
            assert(v.type.value == arg_t.value);
            vec_push(&ctx->eval.stack, v);
        }
        link = &types->data[link->u.func.out.value];
    }
    assert((n - 1) == T_argc && "argument overflow");

    const value_t *argv = &ctx->eval.stack.data[ofs];
    value_t ret = func.type.value <= ctx->state.types.end_intrinsics
                  ? func.u.intrinsic.value(ctx, argv)
                  : func_call(ctx, func, argv);
    for (size_t i = 1; i < n; ++i) {
        vec_pop(&ctx->eval.stack);
    }
    return ret;
}
