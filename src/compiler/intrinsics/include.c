#include <system.h>
#include "include.h"

#include "_.h"
#include <compiler/env.h>
#include <lib/fs.h>
#include <lib/stdio.h>
#include <compiler/compilation.h>

INTRINSIC_IMPL(include, ((type_id[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &Slice_data(&argv)[0];
    const node_t *it = compilation_node(env.compilation, arg_args->u.expr.value);
    assert(it->kind == NODE_STRING && "argument is string literal");
    String path = it->u.string.value;
    compilation_file_ref next = compilation_include(env.compilation, fs_path_from(path));
    compilation_begin(env.compilation, next, env);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
