#include <system.h>
#include "puts.h"

#include <lib/stdio.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(debug_puts, ((type_id[]) {
        types->t_string,
        types->t_unit,
}))
{
    const value_t *arg_val = Slice_at(&argv, 0);
    const String val = arg_val->u.string.value;
    fprintf_s(env.out, val);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
