#pragma once

#include "compilation-fwd.h"

#include <lib/string.h>
#include <lib/vector.h>

#include <parser/token.h>
#include <parser/node.h>
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
    Allocator *allocator;
    FilePath path;
    uint8_t *content;
    Vector(token_t) tokens;
    Vector(node_t) nodes;
    compilation_node_ref entry;
};

void compilation_file_t_delete(compilation_file_t *self);

typedef struct {
    Allocator *allocator;
    compilation_file_ref file;
    Buffer *content;
    File *out;
    String ext;
    size_t flags;
} compile_file;

compile_file compile_file_new(Allocator *allocator, compilation_file_ref file, String ext, size_t flags);

void compile_file_delete(compile_file *self);

Slice_instantiate(compile_file);
Vector_instantiate(compile_file);

compilation_file_ref compilation_include(Allocator *allocator, compilation_t *self, FileSystem *fs, FilePath path);

void compilation_begin(Allocator *allocator, compilation_t *self, compilation_file_ref file, Env env);
