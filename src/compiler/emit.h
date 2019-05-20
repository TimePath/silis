#pragma once

#include "targets/_.h"

typedef struct {
    Interpreter *interpreter;
    struct Target *target;
} emit_input;

typedef struct {
    Vector(compile_file) files;
} emit_output;

emit_output do_emit(emit_input in);
