#pragma once

#include <interpreter/intrinsic.h>

INTRINSIC(func);

value_t func_call(Interpreter *interpreter, value_t func, const Slice(value_t) argv, compilation_node_ref self);

void func_args_types(Interpreter *interpreter, nodelist iter, TypeRef out[]);

void func_args_names(Interpreter *interpreter, nodelist iter, String out[]);
