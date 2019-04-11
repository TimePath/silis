#pragma once

#include "compilation-fwd.h"

#include <lib/string.h>
#include <lib/vector.h>

#include "token-fwd.h"
#include "node-fwd.h"
#include "env.h"

struct compilation_s {
    File *debug;
    Vector(compilation_file_ptr_t) files;
    const struct {
        bool print_lex : 1;
        bool print_parse : 1;
        bool print_eval: 1;
        uint8_t _padding : 5;
    } flags;
    PADDING(7)
};

struct compilation_file_s {
    FilePath path;
    uint8_t *content;
    Vector(token_t) tokens;
    Vector(node_t) nodes;
    compilation_node_ref entry;
};

void compilation_file_t_delete(compilation_file_t *self);

typedef struct {
    compilation_file_ref file;
    Buffer *content;
    File *out;
} compile_file;

compile_file compile_file_new(compilation_file_ref file);

void compile_file_delete(compile_file *self);

Slice_instantiate(compile_file);
Vector_instantiate(compile_file);

compilation_file_ref compilation_include(compilation_t *self, FilePath path);

void compilation_begin(compilation_t *self, compilation_file_ref file, Env env);
