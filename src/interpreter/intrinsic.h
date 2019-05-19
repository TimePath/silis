#pragma once

#include <lib/macro.h>

#include "env.h"
#include "types.h"
#include "value.h"

typedef struct Intrinsic {
    void *name;

    TypeRef (*load)(Types *types);

    value_t (*call)(Env env, compilation_node_ref self, const Slice(value_t) argv);
} Intrinsic;

value_t Intrinsic_call(Intrinsic *self, Env env, compilation_node_ref node, Slice(value_t) argv);

#define INTRINSIC(id) \
extern Intrinsic CAT2(intrin_, id) \
/**/

#define INTRINSIC_IMPL(id, T) \
static TypeRef CAT3(intrin_, id, _load)(Types *types); \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv); \
Intrinsic CAT2(intrin_, id) = { \
    .name = #id, \
    .load = CAT3(intrin_, id, _load), \
    .call = CAT3(intrin_, id, _call), \
}; \
static TypeRef CAT3(intrin_, id, _load)(Types *types) { \
    return Types_register_func(types, Slice_of(TypeRef, T)); \
} \
static value_t CAT3(intrin_, id, _call)(Env env, compilation_node_ref self, Slice(value_t) argv) \
/**/
