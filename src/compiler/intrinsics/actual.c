#include <prelude.h>
#include "actual.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>
#include <interpreter/value.h>
#include <parser/node.h>

INTRINSIC_IMPL(actual, ((Array(Ref(Type), 4)) {
        types->t_expr, types->t_string, types->t_string,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_target = Slice_at(&argv, 1);
    const Value *arg_val = Slice_at(&argv, 2);

    const Node *node_name = Interpreter_lookup_file_node(interpreter, arg_name->u.Expr);
    assert(node_name->kind.val == Node_Atom);
    String name = node_name->u.Atom.value;

    String target = arg_target->u.String;

    String code = arg_val->u.String;

    Symbol *symbol;
    bool found = Symbols_lookup_mut(interpreter->symbols, name, &symbol);
    (void) found;
    assert(found);
    const Value val = symbol->value;
    assert(val.flags.expect && val.kind.val == Value_Integral);
    Interpreter_expectation_set(interpreter, val.u.Integral, target, code);

    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
