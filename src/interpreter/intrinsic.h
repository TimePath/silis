#pragma once

#include <lib/macro.h>

#include "interpreter.h"
#include "types.h"
#include "value.h"

typedef struct Intrinsic {
    void *name;

    TypeRef (*load)(Types *types);

    Value (*call)(Interpreter *interpreter, InterpreterFileNodeRef self, Slice(Value) argv);
} Intrinsic;

Value Intrinsic_call(Intrinsic *self, Interpreter *interpreter, InterpreterFileNodeRef node, Slice(Value) argv);

#define INTRINSIC(id) \
extern Intrinsic CAT2(intrin_, id) \
/**/

#define INTRINSIC_IMPL(id, T) \
static TypeRef CAT3(intrin_, id, _load)(Types *types); \
static Value CAT3(intrin_, id, _call)(Interpreter *interpreter, InterpreterFileNodeRef self, Slice(Value) argv); \
Intrinsic CAT2(intrin_, id) = { \
    .name = #id, \
    .load = CAT3(intrin_, id, _load), \
    .call = CAT3(intrin_, id, _call), \
}; \
static TypeRef CAT3(intrin_, id, _load)(Types *types) { \
    return Types_register_func(types, Slice_of(TypeRef, T)); \
} \
static Value CAT3(intrin_, id, _call)(Interpreter *interpreter, InterpreterFileNodeRef self, Slice(Value) argv) \
/**/
