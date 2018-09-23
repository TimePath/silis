#pragma once

#include <compiler/env.h>

typedef struct {
    Env env;
    FILE *out;
} compile_input;

typedef void compile_output;

compile_output do_compile(compile_input in);
