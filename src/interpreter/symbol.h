#pragma once

#include "interpreter.h"
#include "type.h"
#include "value.h"

typedef struct {
    Ref(InterpreterFile) file;
    Ref(Type) type; // could be removed if values could be tagged as undefined
    Value value;
    struct {
        /// interpreter variable (function call)
        bool eval : 1;
        BIT_PADDING(uint8_t, 7)
    } flags;
    PADDING(7)
} Symbol;
