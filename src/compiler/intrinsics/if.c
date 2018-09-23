#include <system.h>
#include "if.h"

#include "_.h"

INTRINSIC_IMPL(if, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    (void) env;
    (void) argv;
    assert(false);
    return (value_t) {.type = env.types->t_unit};
}
