#include <system.h>
#include "intrinsic.h"

value_t Intrinsic_call(Intrinsic *self, Interpreter *interpreter, compilation_node_ref node, Slice(value_t) argv)
{
    return self->call(interpreter, node, argv);
}
