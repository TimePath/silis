#include <prelude.h>
#include "func.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

static void types_func_args_types(Interpreter *interpreter, NodeList iter, TypeRef out[]);

INTRINSIC_IMPL(types_func, ((TypeRef[]) {
        types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = interpreter->allocator;
    const Value *arg_args = Slice_at(&argv, 0);

    NodeList children = NodeList_iterator(interpreter, arg_args->u.expr.value);
    const size_t argc = children._n;
    assert(argc >= 2 && "has enough arguments");
    TypeRef *Ts = new_arr(TypeRef, argc);
    types_func_args_types(interpreter, children, Ts);
    TypeRef T = Types_register_func(interpreter->types, Slice_of_n(TypeRef, Ts, argc));
    free(Ts);
    return (Value) {
            .type = interpreter->types->t_type,
            .node = self,
            .u.type.value = T,
    };
}

static void types_func_args_types(Interpreter *interpreter, NodeList iter, TypeRef out[])
{
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef it = Interpreter_lookup_node_ref(interpreter, ref);
        const Value v = eval_node(interpreter, it);
        TypeRef T = v.type;
        if (T.value == interpreter->types->t_unit.value) {
            out[i] = interpreter->types->t_unit;
            continue;
        }
        assert(T.value == interpreter->types->t_type.value && "argument is a type");
        out[i] = v.u.type.value;
    }
}
