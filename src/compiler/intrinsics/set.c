#include <system.h>
#include <interpreter/env.h>
#include "set.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(set, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit
}))
{
    const value_t *arg_name = Slice_at(&argv, 0);
    const value_t *arg_val = Slice_at(&argv, 1);

    const Node *name = compilation_node(env.compilation, arg_name->u.expr.value);
    assert(name->kind == Node_Atom);
    compilation_node_ref val = arg_val->u.expr.value;

    const value_t v = eval_node(env, val);
    Symbol entry;
    bool found = Symbols_lookup(env.symbols, name->u.Atom.value, &entry);
    (void) found;
    assert(found && "symbol is declared");
    assert(Types_assign(env.types, v.type, entry.type) && "is assignable");
    entry.value = v;
    Symbols_define(env.symbols, name->u.Atom.value, entry);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
