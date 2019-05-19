#include <system.h>
#include "intrinsic.h"

value_t Intrinsic_call(Intrinsic *self, Env env, compilation_node_ref node, Slice(value_t) argv)
{
    return self->call(env, node, argv);
}
