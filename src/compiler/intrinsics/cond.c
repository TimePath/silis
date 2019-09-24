#include <prelude.h>
#include "cond.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(cond, ((Ref(Type)[]) {
        types->t_expr, types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    (void) self;
    const Value *arg_predicate = Slice_at(&argv, 0);
    const Value *arg_true = Slice_at(&argv, 1);
    const Value *arg_false = Slice_at(&argv, 2);

    InterpreterFileNodeRef predicate = arg_predicate->u.expr.value;
    InterpreterFileNodeRef node_true = arg_true->u.expr.value;
    InterpreterFileNodeRef node_false = arg_false->u.expr.value;
    const Value ret = eval_node(interpreter, predicate);
    return eval_node(interpreter, ret.u.integral.value != 0 ? node_true : node_false);
}
