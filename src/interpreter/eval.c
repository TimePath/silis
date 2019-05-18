#include <system.h>
#include "eval.h"

#include "value.h"

typedef struct {
    Env env;
    Vector(value_t) stack;
} eval_ctx;

#define eval_ctx_new(_env) ((eval_ctx) { \
    .env = _env, \
    .stack = Vector_new(_env.allocator), \
})

static value_t do_eval_list_block(eval_ctx *ctx, compilation_node_ref it);

static value_t do_eval_node(eval_ctx *ctx, compilation_node_ref it);

eval_output do_eval(eval_input in)
{
    eval_ctx ctx = eval_ctx_new(in.env);
    do_eval_list_block(&ctx, in.entry);
}

value_t eval_node(Env env, compilation_node_ref it)
{
    eval_ctx ctx = eval_ctx_new(env);
    return do_eval_node(&ctx, it);
}

value_t eval_list_block(Env env, compilation_node_ref it)
{
    eval_ctx ctx = eval_ctx_new(env);
    return do_eval_list_block(&ctx, it);
}

static value_t do_eval_list_block(eval_ctx *ctx, compilation_node_ref it)
{
    assert(compilation_node(ctx->env.compilation, it)->kind == NODE_LIST_BEGIN);
    value_t ret = (value_t) {.type = ctx->env.types->t_unit, .node = it};
    nodelist iter = nodelist_iterator(ctx->env.compilation, it);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref stmt = node_deref(ctx->env.compilation, ref);
        ret = do_eval_node(ctx, stmt);
    }
    return ret;
}

static value_t do_eval_node(eval_ctx *ctx, compilation_node_ref it)
{
    const node_t *node = compilation_node(ctx->env.compilation, it);
    assert(node->kind != NODE_REF);
    if (node->kind != NODE_LIST_BEGIN) {
        return value_from(ctx->env, it);
    }
    nodelist children = nodelist_iterator(ctx->env.compilation, it);
    const size_t n = children._n;
    if (!n) {
        // ()
        return (value_t) {.type = ctx->env.types->t_unit, .node = it};
    }
    compilation_node_ref ref;
    nodelist_next(&children, &ref);
    if (n == 1) {
        // (expr)
        return do_eval_node(ctx, node_deref(ctx->env.compilation, ref));
    }
    // (f args...)
    const value_t func = do_eval_node(ctx, node_deref(ctx->env.compilation, ref));
    const type_t *funcType = type_lookup(ctx->env.types, func.type);
    assert(funcType->kind == TYPE_FUNCTION && "function is function");

    bool abstract = func.flags.abstract;
    const size_t ofs = Vector_size(&ctx->stack);
    const type_id expr_t = ctx->env.types->t_expr;
    size_t T_argc = 0;
    type_id T = func.type;
    for (const type_t *argType = funcType; argType->kind == TYPE_FUNCTION; T = argType->u.func.out, argType = type_lookup(ctx->env.types, T)) {
        assert((n - 1) > T_argc && "argument underflow");
        compilation_node_ref arg = nodelist_get(&children, ++T_argc);
        arg = node_deref(ctx->env.compilation, arg);
        const type_id arg_t = argType->u.func.in;
        value_t v;
        if (arg_t.value == expr_t.value) {
            v = (value_t) {
                    .type = expr_t,
                    .node = arg,
                    .u.expr.value = arg,
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
        const value_t *argv = Vector_at(&ctx->stack, ofs);
        ret = func_call(ctx->env, func, (Slice(value_t)) {._begin = argv, ._end = argv + n,}, it);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}

static void func_args_load(Env env, compilation_node_ref arglist, Slice(value_t) argv);

value_t func_call(Env env, value_t func, const Slice(value_t) argv, compilation_node_ref it)
{
    if (func.flags.intrinsic) {
        return Intrinsic_call(func.u.intrinsic.value, env, it, argv);
    }
    sym_push(env.symbols);
    compilation_node_ref body = func.u.func.value;
    compilation_node_ref arglist = func.u.func.arglist;
    func_args_load(env, arglist, argv);
    const value_t ret = eval_node(env, body);
    sym_pop(env.symbols);
    return ret;
}

static void func_args_load(Env env, compilation_node_ref arglist, const Slice(value_t) argv)
{
    nodelist iter = nodelist_iterator(env.compilation, arglist);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        nodelist children = nodelist_iterator(env.compilation, it);
        nodelist_next(&children, NULL);
        compilation_node_ref id;
        if (nodelist_next(&children, &id)) {
            const node_t *idNode = compilation_node(env.compilation, id);
            assert(idNode->kind == NODE_ATOM && "argument is a name");

            const value_t *v = Slice_at(&argv, i);
            sym_def(env.symbols, idNode->u.atom.value, (sym_t) {
                    .file = {0},
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}

