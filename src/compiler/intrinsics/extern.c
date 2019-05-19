#include <system.h>
#include "extern.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(extern, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_name = Slice_at(&argv, 0);
    const value_t *arg_type = Slice_at(&argv, 1);

    const Node *name = compilation_node(env.compilation, arg_name->u.expr.value);
    assert(name->kind == Node_Atom);
    compilation_node_ref val = arg_type->u.expr.value;

    const value_t v = eval_node(env, val);
    assert(v.type.value == env.types->t_type.value && "argument is a type");
    type_id T = v.u.type.value;
    sym_def(env.symbols, name->u.Atom.value, (sym_t) {
            .file = self.file,
            .type = T,
            .value = {.type = T, .node = self, .flags.abstract = true, .flags.native = true,},
    });
    return (value_t) {.type = env.types->t_unit, .node = self};
}
