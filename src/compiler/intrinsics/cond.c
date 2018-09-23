#include <system.h>
#include "cond.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(cond, ((type_id[]) {
        types->t_expr, types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_predicate = &Slice_data(&argv)[0];
    const value_t *arg_true = &Slice_data(&argv)[1];
    const value_t *arg_false = &Slice_data(&argv)[2];

    const node_t *predicate = node_get(env.nodes, arg_predicate->u.expr.value);
    const node_t *node_true = node_get(env.nodes, arg_true->u.expr.value);
    const node_t *node_false = node_get(env.nodes, arg_false->u.expr.value);
    const value_t ret = eval_node(env, predicate);
    return eval_node(env, ret.u.integral.value != 0 ? node_true : node_false);
}
