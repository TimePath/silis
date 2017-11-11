#include "../_.h"

#include "../../lib/number.h"

#include <assert.h>
#include <alloca.h>
#include <stdio.h>

INTRINSIC(puti, ((type_id[]) {
        ctx->state.types.t_int,
        ctx->state.types.t_unit,
})) {
    const value_t *arg_val = &argv[0];
    assert(arg_val->type.value == ctx->state.types.t_int.value);
    const size_t val = arg_val->u.integral.value;
    const size_t n = num_size_base10(val);
    char *buf = alloca(n + 1);
    sprintf(buf, "%lu", val);
    buf[n] = 0;
    printf(STR_PRINTF, STR_PRINTF_PASS(STR(buf)));
    return (value_t) {.type = ctx->state.types.t_unit};
}
