#pragma once

#include "interpreter.h"
#include "type.h"
#include "value.h"

typedef struct {
    InterpreterFileRef file;
    TypeRef type; // could be removed if values could be tagged as undefined
    Value value;
    struct {
        /// interpreter variable (function call)
        bool eval : 1;
        uint8_t _padding : 7;
    } flags;
    PADDING(7)
} Symbol;
