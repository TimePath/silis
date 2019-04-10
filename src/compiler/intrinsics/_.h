#pragma once

#include <lib/macro.h>

#include <compiler/intrinsic.h>
#include <compiler/types.h>
#include <compiler/env.h>

#define INTRINSIC(id) \
extern Intrinsic CAT2(intrin_, id) \
/**/

#define INTRINSIC_IMPL(id, T) \
static type_id CAT3(intrin_, id, _load)(types_t *types); \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv); \
Intrinsic CAT2(intrin_, id) = { \
    .load = CAT3(intrin_, id, _load), \
    .call = CAT3(intrin_, id, _call), \
}; \
static type_id CAT3(intrin_, id, _load)(types_t *types) { \
    return type_func_new(types, T, ARRAY_LEN(T)); \
} \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv) \
/**/
