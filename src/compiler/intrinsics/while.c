#include <prelude.h>
#include "while.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(while, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_predicate = Slice_at(&argv, 0);
    const Value *arg_body = Slice_at(&argv, 1);

    InterpreterFileNodeRef predicate = arg_predicate->u.expr.value;
    InterpreterFileNodeRef node_body = arg_body->u.expr.value;
    while (true) {
        const Value ret = eval_node(interpreter, predicate);
        if (ret.u.integral.value != 0) {
            eval_node(interpreter, node_body);
        } else {
            break;
        }
    }
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
