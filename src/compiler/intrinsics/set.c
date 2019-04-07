#include <system.h>
#include <compiler/env.h>
#include "set.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(set, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit
}))
{
    const value_t *arg_name = Slice_at(&argv, 0);
    const value_t *arg_val = Slice_at(&argv, 1);

    const node_t *name = compilation_node(env.compilation, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    compilation_node_ref val = arg_val->u.expr.value;

    const value_t v = eval_node(env, val);
    sym_t entry;
    bool found = sym_lookup(env.symbols, name->u.atom.value, &entry);
    (void) found;
    assert(found && "symbol is declared");
    assert(type_assignable_to(env.types, v.type, entry.type) && "is assignable");
    entry.value = v;
    sym_def(env.symbols, name->u.atom.value, entry);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
