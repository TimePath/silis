#include <system.h>
#include "emit.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(emit, ((TypeRef[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const Value *arg_args = Slice_at(&argv, 0);
    NodeList iter = NodeList_iterator(interpreter, arg_args->u.expr.value);
    InterpreterFileNodeRef ref;
    while (NodeList_next(&iter, &ref)) {
        const Node *node = Interpreter_lookup_file_node(interpreter, ref);
        (void) node;
        assert(node->kind.val == Node_String && "argument is string literal");
    }
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
