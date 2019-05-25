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
    const Value *arg_args = Slice_at(&argv, 0);
    const Node *it = Interpreter_lookup_file_node(interpreter, arg_args->u.expr.value);
    assert(it->kind.val == Node_String && "argument is string literal");
    String path = it->u.String.value;
    InterpreterFileRef next = Interpreter_load(interpreter, interpreter->fs_in, fs_path_from(allocator, path));
    Interpreter_eval(interpreter, next);
    return (Value) {.type = interpreter->types->t_unit, .node = self};
}
