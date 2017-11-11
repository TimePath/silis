#pragma once

#include "../lib/macro.h"
#include "../ctx.h"

#define INTRINSIC_(name, id, T) \
static value_t intrinsic_##id(ctx_t *ctx, value_t *argv); \
static void setup(ctx_t *ctx) { \
    ctx_init_intrinsic(ctx, STR("#"#name), type_func_new(ctx, T, ARRAY_LEN(T)), intrinsic_##id); \
} \
STATIC_INIT { ctx_register_t r = setup; vec_push(&intrinsics, r); } \
static value_t intrinsic_##id(ctx_t *ctx, value_t *argv)

#define INTRINSIC(name, T) INTRINSIC_(name, name, T)
