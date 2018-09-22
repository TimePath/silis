#include <system.h>
#include "extern.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(extern, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_name = &argv[0];
    const value_t *arg_val = &argv[1];

    const node_t *name = node_get(env.nodes, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    const node_t *val = node_get(env.nodes, arg_val->u.expr.value);

    const value_t v = eval_node(env, val);
    assert(v.type.value == env.types->t_type.value);
    sym_def(env.symbols, name->u.atom.value, (sym_t) {
            .type = v.u.type.value,
            .flags.native = true,
    });
    return (value_t) {.type = env.types->t_unit};
}
