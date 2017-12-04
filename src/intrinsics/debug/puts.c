#include "../../system.h"

#include "../_.h"
#include "../../lib/stdio.h"

INTRINSIC(puts, ((type_id[]) {
        ctx->state.types.t_string,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_val = &argv[0];
    string_view_t val = arg_val->u.string.value;
    fprintf_s(stdout, val);
    return (value_t) {.type = ctx->state.types.t_unit};
}
