#include <system.h>
#include "plus.h"

#include "_.h"

INTRINSIC_IMPL(plus, ((type_id[]) {
        types->t_int, types->t_int,
        types->t_int
}))
{
    const size_t a = Slice_at(&argv, 0)->u.integral.value;
    const size_t b = Slice_at(&argv, 1)->u.integral.value;
    const size_t c = a + b;
    return (value_t) {
            .type = env.types->t_int,
            .node = self,
            .u.integral.value = c,
    };
}
