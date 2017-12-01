#include "../_.h"

INTRINSIC(puts, ((type_id[]) {
        ctx->state.types.t_string,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_val = &argv[0];
    string_view_t val = arg_val->u.string.value;
    printf(STR_PRINTF, STR_PRINTF_PASS(val));
    return (value_t) {.type = ctx->state.types.t_unit};
}
