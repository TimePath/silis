#include <system.h>
#include "eval.h"

#include <compiler/value.h>
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
    value_t ret = (value_t) {.type = ctx->env.types->t_unit, .node = it};
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
        return (value_t) {.type = ctx->env.types->t_unit, .node = it};
    }
    if (n == 1) {
        // (expr)
        return do_eval_node(ctx, node_deref(&Slice_data(&children)[0], ctx->env.nodes));
    }
    // (f args...)
    const value_t func = do_eval_node(ctx, node_deref(&Slice_data(&children)[0], ctx->env.nodes));
    const type_t *funcType = type_lookup(ctx->env.types, func.type);
    assert(funcType->kind == TYPE_FUNCTION && "function is function");

    bool abstract = func.flags.abstract;
    const size_t ofs = Vector_size(&ctx->stack);
    const type_id expr_t = ctx->env.types->t_expr;
    size_t T_argc = 0;
    type_id T = func.type;
    for (const type_t *argType = funcType; argType->kind == TYPE_FUNCTION; T = argType->u.func.out, argType = type_lookup(ctx->env.types, T)) {
        assert((n - 1) > T_argc && "argument underflow");
        const node_t *arg = node_deref(&Slice_data(&children)[++T_argc], ctx->env.nodes);
        const type_id arg_t = argType->u.func.in;
        value_t v;
        if (arg_t.value == expr_t.value) {
            v = (value_t) {
                    .type = expr_t,
                    .node = arg,
                    .u.expr.value = node_ref(arg, ctx->env.nodes),
            };
        } else {
            v = do_eval_node(ctx, arg);
            assert(type_assignable_to(ctx->env.types, v.type, arg_t) && "argument matches declared type");
            assert((func.flags.intrinsic ? !v.flags.abstract : true) && "argument is not abstract");
        }
        abstract = abstract || v.flags.abstract;
        Vector_push(&ctx->stack, v);
    }
    assert((n - 1) == T_argc && "argument count matches declared argument count");

    value_t ret;
    if (abstract) {
        ret = (value_t) {
            .type = T,
            .node = it,
            .flags.abstract = true,
        };
    } else {
        const value_t *argv = &Vector_data(&ctx->stack)[ofs];
        ret = func_call(ctx->env, func, (Slice(value_t)) {._begin = argv, ._end = argv + n,}, it);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}
