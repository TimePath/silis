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
    Allocator *allocator = env.allocator;
    const value_t *arg_args = Slice_at(&argv, 0);
    const node_t *it = compilation_node(env.compilation, arg_args->u.expr.value);
    assert(it->kind == NODE_STRING && "argument is string literal");
    String path = it->u.string.value;
    compilation_file_ref next = compilation_include(allocator, env.compilation, env.fs_in, fs_path_from(allocator, path));
    compilation_begin(allocator, env.compilation, next, env);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
