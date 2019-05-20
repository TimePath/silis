#include <system.h>
#include "set.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(set, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_val = Slice_at(&argv, 1);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.expr.value);
    assert(name->kind == Node_Atom);
    InterpreterFileNodeRef val = arg_val->u.expr.value;

    const Value v = eval_node(interpreter, val);
    Symbol entry;
    bool found = Symbols_lookup(interpreter->symbols, name->u.Atom.value, &entry);
    (void) found;
    assert(found && "symbol is declared");
    assert(Types_assign(interpreter->types, v.type, entry.type) && "is assignable");
    entry.value = v;
    Symbols_define(interpreter->symbols, name->u.Atom.value, entry);
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
