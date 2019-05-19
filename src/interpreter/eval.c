#include <system.h>
#include "eval.h"

#include "value.h"

typedef struct {
    Interpreter *interpreter;
    Vector(value_t) stack;
} eval_ctx;

#define eval_ctx_new(_interpreter) ((eval_ctx) { \
    .interpreter = _interpreter, \
    .stack = Vector_new(_interpreter->allocator), \
})

static value_t do_eval_list_block(eval_ctx *ctx, compilation_node_ref it);

static value_t do_eval_node(eval_ctx *ctx, compilation_node_ref it);

eval_output do_eval(eval_input in)
{
    eval_ctx ctx = eval_ctx_new(in.interpreter);
    do_eval_list_block(&ctx, in.entry);
}

value_t eval_node(Interpreter *interpreter, compilation_node_ref it)
{
    eval_ctx ctx = eval_ctx_new(interpreter);
    return do_eval_node(&ctx, it);
}

value_t eval_list_block(Interpreter *interpreter, compilation_node_ref it)
{
    eval_ctx ctx = eval_ctx_new(interpreter);
    return do_eval_list_block(&ctx, it);
}

static value_t do_eval_list_block(eval_ctx *ctx, compilation_node_ref it)
{
    assert(compilation_node(ctx->interpreter, it)->kind == Node_ListBegin);
    value_t ret = (value_t) {.type = ctx->interpreter->types->t_unit, .node = it};
    nodelist iter = nodelist_iterator(ctx->interpreter, it);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref stmt = node_deref(ctx->interpreter, ref);
        ret = do_eval_node(ctx, stmt);
    }
    return ret;
}

static value_t do_eval_node(eval_ctx *ctx, compilation_node_ref it)
{
    const Node *node = compilation_node(ctx->interpreter, it);
    assert(node->kind != Node_Ref);
    if (node->kind != Node_ListBegin) {
        return value_from(ctx->interpreter, it);
    }
    nodelist children = nodelist_iterator(ctx->interpreter, it);
    const size_t n = children._n;
    if (!n) {
        // ()
        return (value_t) {.type = ctx->interpreter->types->t_unit, .node = it};
    }
    compilation_node_ref ref;
    nodelist_next(&children, &ref);
    if (n == 1) {
        // (expr)
        return do_eval_node(ctx, node_deref(ctx->interpreter, ref));
    }
    // (f args...)
    const value_t func = do_eval_node(ctx, node_deref(ctx->interpreter, ref));
    const Type *funcType = Types_lookup(ctx->interpreter->types, func.type);
    assert(funcType->kind == Type_Function && "function is function");

    bool abstract = func.flags.abstract;
    const size_t ofs = Vector_size(&ctx->stack);
    const TypeRef expr_t = ctx->interpreter->types->t_expr;
    size_t T_argc = 0;
    TypeRef T = func.type;
    for (const Type *argType = funcType; argType->kind == Type_Function; T = argType->u.Function.out, argType = Types_lookup(ctx->interpreter->types, T)) {
        assert((n - 1) > T_argc && "argument underflow");
        compilation_node_ref arg = nodelist_get(&children, ++T_argc);
        arg = node_deref(ctx->interpreter, arg);
        const TypeRef arg_t = argType->u.Function.in;
        value_t v;
        if (arg_t.value == expr_t.value) {
            v = (value_t) {
                    .type = expr_t,
                    .node = arg,
                    .u.expr.value = arg,
            };
        } else {
            v = do_eval_node(ctx, arg);
            assert(Types_assign(ctx->interpreter->types, v.type, arg_t) && "argument matches declared type");
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
        ret = func_call(ctx->interpreter, func, (Slice(value_t)) {._begin = argv, ._end = argv + n,}, it);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}

static void func_args_load(Interpreter *interpreter, compilation_node_ref arglist, Slice(value_t) argv);

value_t func_call(Interpreter *interpreter, value_t func, const Slice(value_t) argv, compilation_node_ref it)
{
    if (func.flags.intrinsic) {
        return Intrinsic_call(func.u.intrinsic.value, interpreter, it, argv);
    }
    Symbols_push(interpreter->symbols);
    compilation_node_ref body = func.u.func.value;
    compilation_node_ref arglist = func.u.func.arglist;
    func_args_load(interpreter, arglist, argv);
    const value_t ret = eval_node(interpreter, body);
    Symbols_pop(interpreter->symbols);
    return ret;
}

static void func_args_load(Interpreter *interpreter, compilation_node_ref arglist, const Slice(value_t) argv)
{
    nodelist iter = nodelist_iterator(interpreter, arglist);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(interpreter, ref);
        nodelist children = nodelist_iterator(interpreter, it);
        nodelist_next(&children, NULL);
        compilation_node_ref id;
        if (nodelist_next(&children, &id)) {
            const Node *idNode = compilation_node(interpreter, id);
            assert(idNode->kind == Node_Atom && "argument is a name");

            const value_t *v = Slice_at(&argv, i);
            Symbols_define(interpreter->symbols, idNode->u.Atom.value, (Symbol) {
                    .file = {0},
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}

