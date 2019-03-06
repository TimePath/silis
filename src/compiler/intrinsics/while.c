#include <system.h>
#include "while.h"

#include "_.h"
#include <compiler/phases/03-eval/eval.h>

INTRINSIC_IMPL(while, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_predicate = &Slice_data(&argv)[0];
    const value_t *arg_body = &Slice_data(&argv)[1];

    compilation_node_ref predicate = arg_predicate->u.expr.value;
    compilation_node_ref node_body = arg_body->u.expr.value;
    while (true) {
        const value_t ret = eval_node(env, predicate);
        if (ret.u.integral.value != 0) {
            eval_node(env, node_body);
        } else {
            break;
        }
    }
    return (value_t) {.type = env.types->t_unit, .node = self};
}
