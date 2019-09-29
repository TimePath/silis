#include <prelude.h>
#include "set.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(set, ((Ref(Type)[3]) {
        types->t_expr, types->t_expr,
        types->t_unit
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_val = Slice_at(&argv, 1);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.Expr);
    assert(name->kind.val == Node_Atom);
    InterpreterFileNodeRef val = arg_val->u.Expr;

    const Value v = eval_node(interpreter, val);
    Symbol entry;
    bool found = Symbols_lookup(interpreter->symbols, name->u.Atom.value, &entry);
    (void) found;
    assert(found && "symbol is declared");
    assert(Types_assign(interpreter->types, v.type, entry.type) && "is assignable");
    entry.value = v;
    Symbols_define(interpreter->symbols, name->u.Atom.value, entry);
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
