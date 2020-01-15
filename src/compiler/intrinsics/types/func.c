#include <prelude.h>
#include "func.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

static void types_func_args_types(Interpreter *interpreter, NodeList iter, Ref(Type) out[]);

INTRINSIC_IMPL(types_func, ((Array(Ref(Type), 2)) {
        types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = interpreter->allocator;
    const Value *arg_args = Slice_at(&argv, 0);

    NodeList children = NodeList_iterator(interpreter, arg_args->u.Expr);
    const size_t argc = children._n;
    assert(argc >= 2 && "has enough arguments");
    Ref(Type) *Ts = new_arr(Ref(Type), argc);
    types_func_args_types(interpreter, children, Ts);
    Ref(Type) T = Types_register_func(interpreter->types, Slice_of_n(Ref(Type), Ts, argc));
    free(Ts);
    return (Value) {
            .type = interpreter->types->t_type,
            .node = self,
            .kind.val = Value_Type,
            .u.Type = T,
    };
}

static void types_func_args_types(Interpreter *interpreter, NodeList iter, Ref(Type) out[])
{
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef it = Interpreter_lookup_node_ref(interpreter, ref);
        const Value v = eval_node(interpreter, it);
        Ref(Type) T = v.type;
        if (Ref_eq(T, interpreter->types->t_unit)) {
            out[i] = interpreter->types->t_unit;
            continue;
        }
        assert(Ref_eq(T, interpreter->types->t_type) && "argument is a type");
        out[i] = v.u.Type;
    }
}
