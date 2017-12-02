#include "../../system.h"

#include "../_.h"

INTRINSIC(puti, ((type_id[]) {
        ctx->state.types.t_int,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_val = &argv[0];
    const size_t val = arg_val->u.integral.value;
    fprintf(stdout, "%zu", val);
    return (value_t) {.type = ctx->state.types.t_unit};
}
