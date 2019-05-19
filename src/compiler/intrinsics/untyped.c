#include <system.h>
#include "untyped.h"

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(untyped, ((TypeRef[]) {
        types->t_string,
        types->t_untyped,
}))
{
    (void) argv;
    assert(false && "is never called");
    return (value_t) {.type = env.types->t_unit, .node = self};
}
