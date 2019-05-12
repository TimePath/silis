#pragma once

#include "type.h"
#include "value.h"

typedef struct {
    compilation_file_ref file;
    type_id type; // could be removed if values could be tagged as undefined
    value_t value;
    struct {
        /// interpreter variable (function call)
        bool eval : 1;
        uint8_t _padding : 7;
    } flags;
    PADDING(7)
} sym_t;
