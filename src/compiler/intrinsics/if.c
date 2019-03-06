#include <system.h>
#include "if.h"

#include "_.h"
#include <compiler/phases/03-eval/eval.h>

INTRINSIC_IMPL(if, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_predicate = &Slice_data(&argv)[0];
    const value_t *arg_body = &Slice_data(&argv)[1];

    compilation_node_ref predicate = arg_predicate->u.expr.value;
    compilation_node_ref node_body = arg_body->u.expr.value;
    const value_t ret = eval_node(env, predicate);
    if (ret.u.integral.value != 0) {
        eval_node(env, node_body);
    }
    return (value_t) {.type = env.types->t_unit, .node = self};
}
