#include <system.h>
#include "emit.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(emit, ((TypeRef[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = Slice_at(&argv, 0);
    nodelist iter = nodelist_iterator(interpreter, arg_args->u.expr.value);
    compilation_node_ref ref;
    while (nodelist_next(&iter, &ref)) {
        const Node *node = compilation_node(interpreter, ref);
        (void) node;
        assert(node->kind == Node_String && "argument is string literal");
    }
    return (value_t) {.type = interpreter->types->t_unit, .node = self};
}
