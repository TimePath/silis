#pragma once

#include <compiler/compilation.h>
#include <compiler/env.h>
#include <compiler/type.h>

typedef struct Target {
    void (*file_begin)(struct Target *target, Env env, const compile_file *file);
    void (*file_end)(struct Target *target, Env env, const compile_file *file);
    void (*func_forward)(struct Target *target, Env env, const compile_file *file, type_id T, String name);
    void (*func_declare)(struct Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[]);
    void (*var_begin)(struct Target *target, Env env, const compile_file *file, type_id T);
    void (*var_end)(struct Target *target, Env env, const compile_file *file, type_id T);
    void (*identifier)(struct Target *target, Env env, const compile_file *file, String name);
} Target;
