#include <system.h>
#include "untyped.h"

#include "_.h"

INTRINSIC_IMPL(untyped, ((type_id[]) {
        types->t_string,
        types->t_untyped,
}))
{
    (void) argv;
    assert(false && "is never called");
    return (value_t) {.type = env.types->t_unit, .node = self};
}
