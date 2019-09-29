#include <prelude.h>
#include "puti.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(debug_puti, ((Ref(Type)[2]) {
        types->t_int,
        types->t_unit,
}))
{
    const Value *arg_val = Slice_at(&argv, 0);
    const size_t val = arg_val->u.Integral;
    fprintf_zu(interpreter->out, val);
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
