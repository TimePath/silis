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
    nodelist iter = nodelist_iterator(env.compilation, arg_args->u.expr.value);
    compilation_node_ref ref;
    while (nodelist_next(&iter, &ref)) {
        const node_t *node = compilation_node(env.compilation, ref);
        assert(node->kind == NODE_STRING && "argument is string literal");
        fprintf_s(env.prelude, node->u.string.value);
        fprintf_s(env.prelude, STR("\n"));
    }
    return (value_t) {.type = env.types->t_unit, .node = self};
}