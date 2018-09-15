#pragma once

#include <lib/macro.h>

#include "../ctx.h"

#define INTRINSIC_(name, id, T) \
static value_t CAT2(intrinsic_, id)(ctx_t *ctx, const value_t *argv); \
static void CAT2(setup_, id)(ctx_t *ctx) { \
    ctx_init_intrinsic(ctx, STR("#" name), type_func_new(ctx, T, ARRAY_LEN(T)), CAT2(intrinsic_, id)); \
} \
STATIC_INIT(CAT2(intrinsic_, id)) { ctx_register_t r = CAT2(setup_, id); Vector_push(&intrinsics, r); } \
static value_t CAT2(intrinsic_, id)(ctx_t *ctx, const value_t *argv)

#define INTRINSIC(name, T) INTRINSIC_(#name, name, T)
