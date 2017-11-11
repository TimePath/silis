#include "eval.h"

#include <assert.h>

void do_eval(ctx_t *ctx) {
    node_t *it = &ctx->flatten.out.data[ctx->flatten.out.size - 1];
    assert(it->type == NODE_LIST_END);
    while ((--it)->type != NODE_LIST_BEGIN);
    eval_list_block(ctx, it);
}

value_t eval_list_block(ctx_t *ctx, const node_t *it) {
    assert(it->type == NODE_LIST_BEGIN);
    value_t ret = (value_t) {.type = ctx->state.types.t_unit};
    while ((++it)->type != NODE_LIST_END) {
        ret = eval_node(ctx, it);
    }
    return ret;
}

static const node_t *deref(const ctx_t *ctx, const node_t *it) {
    if (it->type == NODE_REF) {
        return &ctx->flatten.out.data[it->u.ref.value];
    }
    return it;
}

value_t eval_node(ctx_t *ctx, const node_t *it) {
    it = deref(ctx, it);
    if (it->type != NODE_LIST_BEGIN) {
        return val_from(ctx, it);
    }
    assert(it->type == NODE_LIST_BEGIN);
    const size_t n = it->u.list.size;
    const node_t *children = n ? it + 1 : NULL;
    const node_t *end = it + n + 1;
    assert(end->type == NODE_LIST_END);

    value_t ret = (value_t) {.type = ctx->state.types.t_unit};
    assert(n != 1 && "no parenthesized expressions yet");
    if (n) {
        const vec_t(type_t) *types = &ctx->state.types.all;
        const value_t func = eval_node(ctx, deref(ctx, &children[0]));
        const type_t *T = &types->data[func.type.value];
        assert(T->type == TYPE_FUNCTION);

        const size_t ofs = ctx->eval.stack.size;
        size_t T_argc = 0;
        // holds return value after this loop
        const type_t *link = T;
        for (; link->type == TYPE_FUNCTION; ++T_argc) {
            const node_t *arg = deref(ctx, &children[T_argc + 1]);
            assert(T_argc < n - 1 && "argument underflow");
            const type_id t = link->u.func.in;
            const type_id expr_t = ctx->state.types.t_expr;
            if (t.value == expr_t.value) {
                value_t v = (value_t) {
                        .type = expr_t,
                        .u.integral.value = arg - ctx->flatten.out.data,
                };
                vec_push(&ctx->eval.stack, v);
            } else {
                value_t v = eval_node(ctx, arg);
                assert(v.type.value == t.value);
                vec_push(&ctx->eval.stack, v);
            }
            link = &types->data[link->u.func.out.value];
        }
        assert(T_argc == n - 1 && "argument overflow");

        if (func.type.value <= ctx->state.types.end_intrinsics) {
            ret = func.u.intrinsic.value(ctx, (ctx->eval.stack.data + ofs));
        } else {
            const node_t *body = &ctx->flatten.out.data[func.u.integral.value];
            ret = eval_node(ctx, body);
        }
        for (size_t i = 1; i < n; ++i) {
            vec_pop(&ctx->eval.stack);
        }
    }
    return ret;
}
