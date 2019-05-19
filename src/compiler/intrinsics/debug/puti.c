#include <system.h>
#include "puti.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(debug_puti, ((TypeRef[]) {
        types->t_int,
        types->t_unit,
}))
{
    const value_t *arg_val = Slice_at(&argv, 0);
    const size_t val = arg_val->u.integral.value;
    fprintf_zu(interpreter->out, val);
    return (value_t) {.type = interpreter->types->t_unit, .node = self};
}
