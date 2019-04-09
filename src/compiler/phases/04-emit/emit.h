#pragma once

#include <compiler/env.h>
#include <compiler/targets/_.h>

typedef struct {
    struct Target_s *target;
    Env env;
} emit_input;

typedef struct {
    Vector(compile_file) files;
} emit_output;

emit_output do_emit(emit_input in);
