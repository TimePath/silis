#include <system.h>
#include "define.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(define, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_name = Slice_at(&argv, 0);
    const value_t *arg_val = Slice_at(&argv, 1);

    const Node *name = compilation_node(env.compilation, arg_name->u.expr.value);
    assert(name->kind == Node_Atom);
    compilation_node_ref val = arg_val->u.expr.value;

    const value_t v = eval_node(env, val);
    sym_def(env.symbols, name->u.Atom.value, (sym_t) {
            .file = self.file,
            .type = v.type,
            .value = v,
    });
    return (value_t) {.type = env.types->t_unit, .node = self};
}
