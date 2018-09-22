#pragma once

#include "_.h"

INTRINSIC(func);

value_t func_call(Env env, value_t func, const value_t *argv);

void func_args_names(Env env, const node_t *args, size_t argc, String out[VLA_LEN(argc)]);
