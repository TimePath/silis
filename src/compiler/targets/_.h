#pragma once

#include <compiler/env.h>
#include <compiler/type.h>

typedef struct {
    struct Target_s *target;
    Env env;
} compile_ctx_t;

typedef struct {
    compilation_file_ref file;
    Buffer *content;
    File *out;
} compile_file;

compile_file compile_file_new(compilation_file_ref file);

void compile_file_delete(compile_file *self);

Slice_instantiate(compile_file);
Vector_instantiate(compile_file);

typedef struct Target_s {
    void (*file_begin)(const compile_ctx_t *ctx, const compile_file *file);
    void (*file_end)(const compile_ctx_t *ctx, const compile_file *file);
    void (*func_forward)(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name);
    void (*func_declare)(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name, const String argnames[]);
    void (*var_begin)(const compile_ctx_t *ctx, const compile_file *file, type_id T);
    void (*var_end)(const compile_ctx_t *ctx, const compile_file *file, type_id T);
    void (*identifier)(const compile_ctx_t *ctx, const compile_file *file, String name);
} Target;
