#include "_.h"

INTRINSIC_(-, minus, ((type_id[]) {
        ctx->state.types.t_int, ctx->state.types.t_int,
        ctx->state.types.t_int
})) {
    const size_t a = (&argv[0])->u.integral.value;
    const size_t b = (&argv[1])->u.integral.value;
    const size_t c = a - b;
    return (value_t) {
            .type = ctx->state.types.t_int,
            .u.integral.value = c,
    };
}
