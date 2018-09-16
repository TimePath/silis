#include <system.h>
#include "puts.h"

#include <lib/stdio.h>

#include "../_.h"

INTRINSIC_IMPL(debug_puts, "puts", ((type_id[]) {
        ctx->state.types.t_string,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_val = &argv[0];
    String val = arg_val->u.string.value;
    fprintf_s(stdout, val);
    return (value_t) {.type = ctx->state.types.t_unit};
}
