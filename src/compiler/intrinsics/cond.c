#include <system.h>
#include "cond.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(cond, ((TypeRef[]) {
        types->t_expr, types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    (void) self;
    const value_t *arg_predicate = Slice_at(&argv, 0);
    const value_t *arg_true = Slice_at(&argv, 1);
    const value_t *arg_false = Slice_at(&argv, 2);

    compilation_node_ref predicate = arg_predicate->u.expr.value;
    compilation_node_ref node_true = arg_true->u.expr.value;
    compilation_node_ref node_false = arg_false->u.expr.value;
    const value_t ret = eval_node(env, predicate);
    return eval_node(env, ret.u.integral.value != 0 ? node_true : node_false);
}
