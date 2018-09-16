#pragma once

#include <lib/macro.h>

#include "../ctx.h"

typedef struct {
    void (*load)(ctx_t *ctx);

    value_t (*call)(ctx_t *ctx, const value_t *argv);
} Intrinsic;

#define INTRINSIC(id) \
extern Intrinsic CAT2(intrin_, id) \
/**/

#define INTRINSIC_IMPL(id, name, T) \
static void CAT3(intrin_, id, _load)(ctx_t *ctx); \
static value_t CAT3(intrin_, id, _call)(ctx_t *ctx, const value_t *argv); \
Intrinsic CAT2(intrin_, id) = { \
    .load = CAT3(intrin_, id, _load), \
    .call = CAT3(intrin_, id, _call), \
}; \
static void CAT3(intrin_, id, _load)(ctx_t *ctx) { \
    ctx_init_intrinsic(ctx, STR("#" name), type_func_new(ctx, T, ARRAY_LEN(T)), CAT2(intrin_, id).call); \
} \
static value_t CAT3(intrin_, id, _call)(ctx_t *ctx, const value_t *argv) \
/**/
