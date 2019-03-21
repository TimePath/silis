#pragma once

#include <compiler/env.h>
#include <compiler/targets/_.h>

typedef struct {
    struct Target_s *target;
    Env env;
} compile_input;

typedef struct {
    Vector(compile_file) files;
} compile_output;

compile_output do_compile(compile_input in);
