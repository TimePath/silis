#include <system.h>
#include "emit.h"

#include "_.h"
#include <lib/stdio.h>
#include <compiler/env.h>

INTRINSIC_IMPL(emit, ((type_id[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &Slice_data(&argv)[0];
    Slice(node_t) children = node_list_children(compilation_node(env.compilation, arg_args->u.expr.value));
    Slice_loop(&children, i) {
        const node_t *it = &Slice_data(&children)[i];
        assert(it->kind == NODE_STRING && "argument is string literal");
        fprintf_s(env.prelude, it->u.string.value);
        fprintf_s(env.prelude, STR("\n"));
    }
    return (value_t) {.type = env.types->t_unit, .node = self};
}
