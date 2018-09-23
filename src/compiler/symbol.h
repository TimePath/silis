#pragma once

#include "type.h"
#include "value.h"

typedef struct {
    type_id type; // could be removed if values could be tagged as undefined
    value_t value;
    struct {
        /// interpreter variable (function call)
        bool eval : 1;
        uint8_t padding : 7;
    } flags;
    uint8_t padding[7];
} sym_t;
