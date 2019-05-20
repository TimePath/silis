#include <system.h>
#include "intrinsic.h"

Value Intrinsic_call(Intrinsic *self, Interpreter *interpreter, InterpreterFileNodeRef node, Slice(Value) argv)
{
    return self->call(interpreter, node, argv);
}
