#include <system.h>
#include "eval.h"

#include "../../intrinsics/func.h"

typedef struct {
    Env env;
    Vector(value_t) stack;
} eval_ctx_t;

static value_t do_eval_list_block(eval_ctx_t *ctx, const node_t *it);

static value_t do_eval_node(eval_ctx_t *ctx, const node_t *it);

eval_output do_eval(eval_input in)
{
    eval_ctx_t ctx = {.env = in.env};
    const node_t *it = &Vector_data(ctx.env.nodes)[Vector_size(ctx.env.nodes) - 1];
    assert(it->kind == NODE_LIST_END);
    while ((--it)->kind != NODE_LIST_BEGIN) {}
    do_eval_list_block(&ctx, it);
}

value_t eval_node(Env env, const node_t *it)
{
    return do_eval_node(&(eval_ctx_t) {.env = env}, it);
}

value_t eval_list_block(Env env, const node_t *it)
{
    return do_eval_node(&(eval_ctx_t) {.env = env}, it);
}

static value_t do_eval_list_block(eval_ctx_t *ctx, const node_t *it)
{
    const Slice(node_t) children = node_list_children(it);
    const size_t n = Slice_size(&children);
    value_t ret = (value_t) {.type = ctx->env.types->t_unit};
    for (size_t i = 0; i < n; ++i) {
        ret = do_eval_node(ctx, &Slice_data(&children)[i]);
    }
    return ret;
}

static value_t do_eval_node(eval_ctx_t *ctx, const node_t *it)
{
    it = node_deref(it, ctx->env.nodes);
    if (it->kind != NODE_LIST_BEGIN) {
        return value_from(ctx->env, it);
    }
    const Slice(node_t) children = node_list_children(it);
    const size_t n = Slice_size(&children);
    if (!n) {
        return (value_t) {.type = ctx->env.types->t_unit};
    }
    if (n == 1) {
        return do_eval_node(ctx, &Slice_data(&children)[0]);
    }
    const value_t func = do_eval_node(ctx, &Slice_data(&children)[0]);
    const type_t *T = type_lookup(ctx->env.types, func.type);
    assert(T->kind == TYPE_FUNCTION);

    const size_t ofs = Vector_size(&ctx->stack);
    const type_id expr_t = ctx->env.types->t_expr;
    size_t T_argc = 0;
    // holds return value after this loop
    const type_t *link = T;
    for (; link->kind == TYPE_FUNCTION; ++T_argc) {
        assert((n - 1) > T_argc && "argument underflow");
        const node_t *arg = &Slice_data(&children)[T_argc + 1];
        const type_id arg_t = link->u.func.in;
        if (arg_t.value == expr_t.value) {
            value_t v = (value_t) {
                    .type = expr_t,
                    .u.expr.value = node_ref(node_deref(arg, ctx->env.nodes), ctx->env.nodes),
            };
            Vector_push(&ctx->stack, v);
        } else {
            value_t v = do_eval_node(ctx, arg);
            assert(v.type.value == arg_t.value);
            Vector_push(&ctx->stack, v);
        }
        link = type_lookup(ctx->env.types, link->u.func.out);
    }
    assert((n - 1) == T_argc && "argument overflow");

    const value_t *argv = &Vector_data(&ctx->stack)[ofs];
    value_t ret = func_call(ctx->env, func, argv);
    for (size_t i = 1; i < n; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}
