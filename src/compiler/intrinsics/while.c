#include <system.h>
#include "while.h"

#include "_.h"

INTRINSIC_IMPL(while, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    (void) env;
    (void) argv;
    assert(false);
    return (value_t) {.type = env.types->t_unit};
}
