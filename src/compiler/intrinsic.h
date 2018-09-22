#pragma once

#include "types.h"
#include "value.h"

typedef struct {
    type_id (*load)(types_t *types);

    value_t (*call)(Env env, const value_t *argv);
} Intrinsic;
