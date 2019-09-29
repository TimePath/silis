#include <prelude.h>
#include "extern.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(extern, ((Ref(Type)[3]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_type = Slice_at(&argv, 1);

    const Node *name = Interpreter_lookup_file_node(interpreter, arg_name->u.Expr);
    assert(name->kind.val == Node_Atom);
    InterpreterFileNodeRef val = arg_type->u.Expr;

    const Value v = eval_node(interpreter, val);
    assert(Ref_eq(v.type, interpreter->types->t_type) && "argument is a type");
    Ref(Type) T = v.u.Type;
    Symbols_define(interpreter->symbols, name->u.Atom.value, (Symbol) {
            .file = self.file,
            .type = T,
            .value = {.type = T, .node = self, .flags = { .abstract = true, .native = true, }},
    });
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
