#pragma once

#include "../ctx.h"

value_t func_call(ctx_t *ctx, value_t func, const value_t *argv);

void func_args_names(const ctx_t *ctx, const node_t *args, size_t argc, string_view_t out[argc]);
