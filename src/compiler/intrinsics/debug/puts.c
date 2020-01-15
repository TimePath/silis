#include <prelude.h>
#include "puts.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(debug_puts, ((Array(Ref(Type), 2)) {
        types->t_string,
        types->t_unit,
}))
{
    const Value *arg_val = Slice_at(&argv, 0);
    const String val = arg_val->u.String;
    fprintf_s(interpreter->out, val);
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
