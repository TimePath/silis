#include <prelude.h>
#include "plus.h"

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(plus, ((TypeRef[]) {
        types->t_int, types->t_int,
        types->t_int
}))
{
    const size_t a = Slice_at(&argv, 0)->u.integral.value;
    const size_t b = Slice_at(&argv, 1)->u.integral.value;
    const size_t c = a + b;
    return (Value) {
            .type = interpreter->types->t_int,
            .node = self,
            .u.integral.value = c,
    };
}
