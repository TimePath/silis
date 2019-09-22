#include <prelude.h>
#include "actual.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(actual, ((TypeRef[]) {
        types->t_expr, types->t_string, types->t_string,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    assert(arg_name->type.value == interpreter->types->t_expr.value);
    const Value *arg_target = Slice_at(&argv, 1);
    assert(arg_target->type.value == interpreter->types->t_string.value);
    const Value *arg_val = Slice_at(&argv, 2);
    assert(arg_val->type.value == interpreter->types->t_string.value);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.expr.value);
    assert(name->kind.val == Node_Atom);

//    InterpreterFileNodeRef val = arg_val->u.expr.value;
//    const Value v = eval_node(interpreter, val);

    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
