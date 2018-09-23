#pragma once

#include "types.h"
#include "value.h"

typedef struct Intrinsic_s {
    type_id (*load)(types_t *types);

    value_t (*call)(Env env, const Slice(value_t) argv);
} Intrinsic;
