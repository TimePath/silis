#pragma once

#include "_.h"

INTRINSIC(func);

value_t func_call(Env env, value_t func, Slice(value_t) argv, compilation_node_ref self);

void func_args_types(Env env, Slice(node_t) args, type_id out[]);

void func_args_names(Env env, Slice(node_t) args, String out[]);
