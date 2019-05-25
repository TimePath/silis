#include <system.h>
#include "define.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(define, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_val = Slice_at(&argv, 1);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.expr.value);
    assert(name->kind.val == Node_Atom);
    InterpreterFileNodeRef val = arg_val->u.expr.value;

    const Value v = eval_node(interpreter, val);
    Symbols_define(interpreter->symbols, name->u.Atom.value, (Symbol) {
            .file = self.file,
            .type = v.type,
            .value = v,
    });
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
