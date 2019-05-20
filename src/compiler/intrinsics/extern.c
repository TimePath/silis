#include <system.h>
#include "extern.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(extern, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_type = Slice_at(&argv, 1);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.expr.value);
    assert(name->kind == Node_Atom);
    InterpreterFileNodeRef val = arg_type->u.expr.value;

    const Value v = eval_node(interpreter, val);
    assert(v.type.value == interpreter->types->t_type.value && "argument is a type");
    TypeRef T = v.u.type.value;
    Symbols_define(interpreter->symbols, name->u.Atom.value, (Symbol) {
            .file = self.file,
            .type = T,
            .value = {.type = T, .node = self, .flags.abstract = true, .flags.native = true,},
    });
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
