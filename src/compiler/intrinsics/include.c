#include <system.h>
#include "include.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <interpreter/interpreter.h>
#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(include, ((TypeRef[]) {
        types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = interpreter->allocator;
    const value_t *arg_args = Slice_at(&argv, 0);
    const Node *it = compilation_node(interpreter, arg_args->u.expr.value);
    assert(it->kind == Node_String && "argument is string literal");
    String path = it->u.String.value;
    compilation_file_ref next = compilation_include(allocator, interpreter, interpreter->fs_in, fs_path_from(allocator, path));
    compilation_begin(allocator, interpreter, next, interpreter);
    return (value_t) {.type = interpreter->types->t_unit, .node = self};
}
