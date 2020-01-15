#include <prelude.h>
#include "untyped.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(untyped, ((Array(Ref(Type), 2)) {
        types->t_string,
        types->t_untyped,
}))
{
    (void) argv;
    assert(false && "is never called");
    return (Value) {
            .type = interpreter->types->t_unit,
            .node = self,
            .kind.val = Value_Opaque,
    };
}
