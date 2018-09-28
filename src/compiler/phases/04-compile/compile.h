#pragma once

#include <compiler/env.h>

typedef struct {
    FILE *out;
    struct Target_s *target;
    Env env;
} compile_input;

typedef void compile_output;

compile_output do_compile(compile_input in);
