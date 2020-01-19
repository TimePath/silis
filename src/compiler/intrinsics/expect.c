#include <prelude.h>
#include "expect.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(expect, ((Array(Ref(Type), 3)) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_name = Slice_at(&argv, 0);
    const Value *arg_type = Slice_at(&argv, 1);

    const Node *node_name = Interpreter_lookup_file_node(interpreter, arg_name->u.Expr);
    assert(node_name->kind.val == Node_Atom);
    String name = node_name->u.Atom.value;

    const Value v = eval_node(interpreter, arg_type->u.Expr);
    assert(Ref_eq(v.type, interpreter->types->t_type) && "argument is a type");
    Ref(Type) T = v.u.Type;

    size_t id = Interpreter_expectation_alloc(interpreter);

    Symbols_define(interpreter->symbols, name, (Symbol) {
            .file = self.file,
            .type = T,
            .value = {
                    .type = T,
                    .node = self,
                    .kind.val = Value_Integral,
                    .u.Integral = id,
                    .flags = {.abstract = true, .expect = true,}
            },
    });
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
