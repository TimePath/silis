#include <prelude.h>
#include "do.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(do, ((Ref(Type)[2]) {
        types->t_expr,
        types->t_unit, // fixme: return type depends on input
}))
{
    (void) self;
    const Value *arg_body = Slice_at(&argv, 0);

    InterpreterFileNodeRef body = arg_body->u.Expr;
    return eval_list_block(interpreter, body);
}
