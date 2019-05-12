#pragma once

#include <interpreter/compilation.h>
#include <interpreter/env.h>

typedef struct {
    Env env;
    struct Target *target;
} emit_input;

typedef struct {
    Vector(compile_file) files;
} emit_output;

emit_output do_emit(emit_input in);
