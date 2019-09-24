#pragma once

#include <interpreter/intrinsic.h>

INTRINSIC(func);

Value func_call(Interpreter *interpreter, Value func, Slice(Value) argv, InterpreterFileNodeRef self);

void func_args_types(Interpreter *interpreter, NodeList iter, Ref(Type) out[]);

void func_args_names(Interpreter *interpreter, NodeList iter, String out[]);
