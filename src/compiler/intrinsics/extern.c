#include <system.h>
#include "extern.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(extern, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_name = &Slice_data(&argv)[0];
    const value_t *arg_type = &Slice_data(&argv)[1];

    const node_t *name = compilation_node(env.compilation, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    compilation_node_ref val = arg_type->u.expr.value;

    const value_t v = eval_node(env, val);
    assert(v.type.value == env.types->t_type.value && "argument is a type");
    type_id T = v.u.type.value;
    sym_def(env.symbols, name->u.atom.value, (sym_t) {
            .type = T,
            .value = {.type = T, .node = self, .flags.abstract = true, .flags.native = true,},
    });
    return (value_t) {.type = env.types->t_unit, .node = self};
}
