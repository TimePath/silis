#pragma once

#include "_.h"

INTRINSIC(func);

value_t func_call(Env env, value_t func, const value_t *argv);

void func_args_types(Env env, Slice(node_t) args, type_id out[]);

void func_args_names(Env env, Slice(node_t) args, String out[]);
