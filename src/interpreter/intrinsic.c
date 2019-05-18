#include <system.h>
#include "intrinsic.h"

value_t Intrinsic_call(Intrinsic *instance, Env env, compilation_node_ref self, Slice(value_t) argv)
{
    return instance->call(env, self, argv);
}
