#pragma once

#include <compiler/env.h>
#include <compiler/type.h>

typedef struct {
    File *out;
    struct Target_s *target;
    Env env;
} compile_ctx_t;

typedef struct Target_s {
    void (*file_begin)(const compile_ctx_t *ctx);
    void (*file_end)(const compile_ctx_t *ctx);
    void (*func_forward)(const compile_ctx_t *ctx, type_id T, String name);
    void (*func_declare)(const compile_ctx_t *ctx, type_id T, String name, const String argnames[]);
    void (*var_begin)(const compile_ctx_t *ctx, type_id T);
    void (*var_end)(const compile_ctx_t *ctx, type_id T);
    void (*identifier)(const compile_ctx_t *ctx, String name);
} Target;
