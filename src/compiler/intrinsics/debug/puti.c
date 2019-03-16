#include <system.h>
#include "puti.h"

#include <lib/stdio.h>

#include "../_.h"

INTRINSIC_IMPL(debug_puti, ((type_id[]) {
        types->t_int,
        types->t_unit,
}))
{
    const value_t *arg_val = &Slice_data(&argv)[0];
    const size_t val = arg_val->u.integral.value;
    fprintf_zu(env.out, val);
    return (value_t) {.type = env.types->t_unit, .node = self};
}
