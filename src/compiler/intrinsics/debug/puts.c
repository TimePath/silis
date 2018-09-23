#include <system.h>
#include "puts.h"

#include <lib/stdio.h>

#include "../_.h"

INTRINSIC_IMPL(debug_puts, ((type_id[]) {
        types->t_string,
        types->t_unit,
}))
{
    const value_t *arg_val = &Slice_data(&argv)[0];
    const String val = arg_val->u.string.value;
    fprintf_s(stdout, val);
    return (value_t) {.type = env.types->t_unit};
}
