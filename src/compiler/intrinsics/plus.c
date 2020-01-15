#include <prelude.h>
#include "plus.h"

#include <interpreter/intrinsic.h>

INTRINSIC_IMPL(plus, ((Array(Ref(Type), 3)) {
        types->t_int, types->t_int,
        types->t_int
}))
{
    const size_t a = Slice_at(&argv, 0)->u.Integral;
    const size_t b = Slice_at(&argv, 1)->u.Integral;
    const size_t c = a + b;
    return (Value) {
            .type = interpreter->types->t_int,
            .node = self,
            .kind.val = Value_Integral,
            .u.Integral = c,
    };
}
