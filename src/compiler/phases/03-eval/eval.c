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
    do_eval_list_block(&ctx, in.entry);
}

value_t eval_node(Env env, const node_t *it)
{
    return do_eval_node(&(eval_ctx_t) {.env = env}, it);
}

value_t eval_list_block(Env env, const node_t *it)
{
    return do_eval_list_block(&(eval_ctx_t) {.env = env}, it);
}

static value_t do_eval_list_block(eval_ctx_t *ctx, const node_t *it)
{
    assert(it->kind == NODE_LIST_BEGIN);
    const Slice(node_t) children = node_list_children(it);
    const size_t n = Slice_size(&children);
    value_t ret = (value_t) {.type = ctx->env.types->t_unit};
    for (size_t i = 0; i < n; ++i) {
        const node_t *stmt = node_deref(&Slice_data(&children)[i], ctx->env.nodes);
        ret = do_eval_node(ctx, stmt);
    }
    return ret;
}

static value_t do_eval_node(eval_ctx_t *ctx, const node_t *it)
{
    assert(it->kind != NODE_REF);
    if (it->kind != NODE_LIST_BEGIN) {
        return value_from(ctx->env, it);
    }
    const Slice(node_t) children = node_list_children(it);
    const size_t n = Slice_size(&children);
    if (!n) {
        // ()
        return (value_t) {.type = ctx->env.types->t_unit};
    }
    if (n == 1) {
        // (expr)
        return do_eval_node(ctx, node_deref(&Slice_data(&children)[0], ctx->env.nodes));
    }
    // (f args...)
    const value_t func = do_eval_node(ctx, node_deref(&Slice_data(&children)[0], ctx->env.nodes));
    const type_t *T = type_lookup(ctx->env.types, func.type);
    assert(T->kind == TYPE_FUNCTION && "function is function");
    assert(!func.flags.native && "function is not native");

    const size_t ofs = Vector_size(&ctx->stack);
    const type_id expr_t = ctx->env.types->t_expr;
    size_t T_argc = 0;
    for (const type_t *link = T; link->kind == TYPE_FUNCTION; link = type_lookup(ctx->env.types, link->u.func.out)) {
        assert((n - 1) > T_argc && "argument underflow");
        const node_t *arg = node_deref(&Slice_data(&children)[++T_argc], ctx->env.nodes);
        const type_id arg_t = link->u.func.in;
        if (arg_t.value == expr_t.value) {
            value_t v = (value_t) {
                    .type = expr_t,
                    .u.expr.value = node_ref(arg, ctx->env.nodes),
            };
            Vector_push(&ctx->stack, v);
        } else {
            value_t v = do_eval_node(ctx, arg);
            assert(v.type.value == arg_t.value && "argument matches declared type");
            Vector_push(&ctx->stack, v);
        }
    }
    assert((n - 1) == T_argc && "argument count matches declared argument count");

    const value_t *argv = &Vector_data(&ctx->stack)[ofs];
    value_t ret = func_call(ctx->env, func, (Slice(value_t)) {._begin = argv, ._end = argv + n,});
    for (size_t i = 0; i < n - 1; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}
