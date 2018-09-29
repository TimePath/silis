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
    const value_t *arg_name = &Slice_data(&argv)[0];
    const value_t *arg_val = &Slice_data(&argv)[1];

    const node_t *name = node_get(env.nodes, arg_name->u.expr.value);
    assert(name->kind == NODE_ATOM);
    const node_t *val = node_get(env.nodes, arg_val->u.expr.value);

    const value_t v = eval_node(env, val);
    sym_t entry;
    bool found = sym_lookup(env.symbols, name->u.atom.value, &entry);
    (void) found;
    assert(found && "symbol is declared");
    assert(entry.type.value == v.type.value && "is assignable");
    entry.value = v;
    sym_def(env.symbols, name->u.atom.value, entry);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
