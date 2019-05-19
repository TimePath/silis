#pragma once

#include <lib/macro.h>

#include "env.h"
#include "types.h"
#include "value.h"

typedef struct Intrinsic {
    void *name;

    type_id (*load)(types_t *types);

    value_t (*call)(Env env, compilation_node_ref self, const Slice(value_t) argv);
} Intrinsic;

value_t Intrinsic_call(Intrinsic *instance, Env env, compilation_node_ref self, Slice(value_t) argv);

#define INTRINSIC(id) \
extern Intrinsic CAT2(intrin_, id) \
/**/

#define INTRINSIC_IMPL(id, T) \
static type_id CAT3(intrin_, id, _load)(types_t *types); \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv); \
Intrinsic CAT2(intrin_, id) = { \
    .name = #id, \
    .load = CAT3(intrin_, id, _load), \
    .call = CAT3(intrin_, id, _call), \
}; \
static type_id CAT3(intrin_, id, _load)(types_t *types) { \
    return type_func_new(types, T, ARRAY_LEN(T)); \
} \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv) \
/**/