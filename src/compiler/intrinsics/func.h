#pragma once

#include <interpreter/intrinsic.h>

INTRINSIC(func);

value_t func_call(Env env, value_t func, const Slice(value_t) argv, compilation_node_ref self);

void func_args_types(Env env, nodelist iter, TypeRef out[]);

void func_args_names(Env env, nodelist iter, String out[]);
