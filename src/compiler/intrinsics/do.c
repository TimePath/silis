#include <system.h>
#include "do.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(do, ((type_id[]) {
        types->t_expr,
        types->t_unit, // fixme: return type depends on input
}))
{
    (void) self;
    const value_t *arg_body = &Slice_data(&argv)[0];

    compilation_node_ref body = arg_body->u.expr.value;
    return eval_list_block(env, body);
}
