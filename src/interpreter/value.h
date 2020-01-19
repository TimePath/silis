#pragma once

#include <lib/vector.h>

#include "interpreter.h"
#include "nodelist.h"
#include "type.h"

#define Value(_, case) \
    case(_, Opaque, struct { PADDING(1); }) \
    case(_, Integral, size_t) \
    case(_, String, String) \
    case(_, Intrinsic, struct Intrinsic *) \
    case(_, Expr, InterpreterFileNodeRef) \
    case(_, Type, Ref(Type)) \
    case(_, Func, struct { \
        InterpreterFileNodeRef value; \
        InterpreterFileNodeRef arglist; \
    }) \
/**/

typedef struct {
    Ref(Type) type;
    InterpreterFileNodeRef node;
    ADT_instantiate_(Value)
    struct {
        /** unknown value */
        bool abstract : 1;
        /** intrinsic, can't be compiled as-is */
        bool intrinsic : 1;
        /** symbol has native implementation (target-specific expression tree) */
        bool expect : 1;
        BIT_PADDING(uint8_t, 5);
    } flags;
    PADDING(7);
} Value;
#undef Value

Slice_instantiate(Value);
Vector_instantiate(Value);

Value Value_from(Interpreter *interpreter, InterpreterFileNodeRef n);
