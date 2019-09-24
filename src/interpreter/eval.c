#include <prelude.h>
#include "eval.h"

#include <lib/misc.h>

#include "intrinsic.h"
#include "value.h"

typedef struct {
    Interpreter *interpreter;
    Vector(Value) stack;
} eval_ctx;

#define eval_ctx_new(_interpreter) ((eval_ctx) { \
    .interpreter = _interpreter, \
    .stack = Vector_new(Value, _interpreter->allocator), \
})

static Value do_eval_list_block(eval_ctx *ctx, InterpreterFileNodeRef it);

static Value do_eval_node(eval_ctx *ctx, InterpreterFileNodeRef it);

eval_output do_eval(eval_input in)
{
    eval_ctx ctx = eval_ctx_new(in.interpreter);
    do_eval_list_block(&ctx, in.entry);
}

Value eval_node(Interpreter *interpreter, InterpreterFileNodeRef it)
{
    eval_ctx ctx = eval_ctx_new(interpreter);
    return do_eval_node(&ctx, it);
}

Value eval_list_block(Interpreter *interpreter, InterpreterFileNodeRef it)
{
    eval_ctx ctx = eval_ctx_new(interpreter);
    return do_eval_list_block(&ctx, it);
}

static Value do_eval_list_block(eval_ctx *ctx, InterpreterFileNodeRef it)
{
    assert(Interpreter_lookup_file_node(ctx->interpreter, it)->kind.val == Node_ListBegin);
    Value ret = (Value) {.type = ctx->interpreter->types->t_unit, .node = it};
    NodeList iter = NodeList_iterator(ctx->interpreter, it);
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef stmt = Interpreter_lookup_node_ref(ctx->interpreter, ref);
        ret = do_eval_node(ctx, stmt);
    }
    return ret;
}

static Value do_eval_node(eval_ctx *ctx, InterpreterFileNodeRef it)
{
    const Node *node = Interpreter_lookup_file_node(ctx->interpreter, it);
    assert(node->kind.val != Node_Ref);
    if (node->kind.val != Node_ListBegin) {
        return Value_from(ctx->interpreter, it);
    }
    NodeList children = NodeList_iterator(ctx->interpreter, it);
    const size_t n = children._n;
    if (!n) {
        // ()
        return (Value) {.type = ctx->interpreter->types->t_unit, .node = it};
    }
    InterpreterFileNodeRef ref;
    NodeList_next(&children, &ref);
    if (n == 1) {
        // (expr)
        return do_eval_node(ctx, Interpreter_lookup_node_ref(ctx->interpreter, ref));
    }
    // (f args...)
    const Value func = do_eval_node(ctx, Interpreter_lookup_node_ref(ctx->interpreter, ref));
    const Type *funcType = Types_lookup(ctx->interpreter->types, func.type);
    assert(funcType->kind.val == Type_Function && "function is function");

    bool abstract = func.flags.abstract;
    const size_t ofs = Vector_size(&ctx->stack);
    const Ref(Type) expr_t = ctx->interpreter->types->t_expr;
    size_t T_argc = 0;
    Ref(Type) T = func.type;
    for (const Type *argType = funcType; argType->kind.val == Type_Function; T = argType->u.Function.out, argType = Types_lookup(ctx->interpreter->types, T)) {
        assert((n - 1) > T_argc && "argument underflow");
        InterpreterFileNodeRef arg = NodeList_get(&children, ++T_argc);
        arg = Interpreter_lookup_node_ref(ctx->interpreter, arg);
        const Ref(Type) arg_t = argType->u.Function.in;
        Value v;
        if (Ref_eq(arg_t, expr_t)) {
            v = (Value) {
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

    Value ret;
    if (abstract) {
        ret = (Value) {
            .type = T,
            .node = it,
            .flags = { .abstract = true, },
        };
    } else {
        const Value *argv = Vector_at(&ctx->stack, ofs);
        ret = func_call(ctx->interpreter, func, (Slice(Value)) {._begin.r = argv, ._end = argv + n,}, it);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        Vector_pop(&ctx->stack);
    }
    return ret;
}

static void func_args_load(Interpreter *interpreter, InterpreterFileNodeRef arglist, Slice(Value) argv);

Value func_call(Interpreter *interpreter, Value func, Slice(Value) argv, InterpreterFileNodeRef it)
{
    if (func.flags.intrinsic) {
        return Intrinsic_call(func.u.intrinsic.value, interpreter, it, argv);
    }
    Symbols_push(interpreter->symbols);
    InterpreterFileNodeRef body = func.u.func.value;
    InterpreterFileNodeRef arglist = func.u.func.arglist;
    func_args_load(interpreter, arglist, argv);
    const Value ret = eval_node(interpreter, body);
    Symbols_pop(interpreter->symbols);
    return ret;
}

static void func_args_load(Interpreter *interpreter, InterpreterFileNodeRef arglist, Slice(Value) argv)
{
    NodeList iter = NodeList_iterator(interpreter, arglist);
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef it = Interpreter_lookup_node_ref(interpreter, ref);
        NodeList children = NodeList_iterator(interpreter, it);
        NodeList_next(&children, NULL);
        InterpreterFileNodeRef id;
        if (NodeList_next(&children, &id)) {
            const Node *idNode = Interpreter_lookup_file_node(interpreter, id);
            assert(idNode->kind.val == Node_Atom && "argument is a name");

            const Value *v = Slice_at(&argv, i);
            Symbols_define(interpreter->symbols, idNode->u.Atom.value, (Symbol) {
                    .file = Ref_null,
                    .type = v->type,
                    .value = *v,
                    .flags = { .eval = true, },
            });
        }
    }
}

