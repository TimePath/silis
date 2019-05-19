#pragma once

#include <lib/string.h>
#include <lib/vector.h>

#include <parser/token.h>
#include <parser/node.h>

#include "env.h"

typedef struct compilation_s compilation_t;
typedef struct compilation_file_s compilation_file_t;

typedef compilation_file_t *compilation_file_ptr_t;

#define compilation_file_ptr_t_delete(self) compilation_file_t_delete(*self)

Slice_instantiate(compilation_file_ptr_t);
Vector_instantiate(compilation_file_ptr_t);

typedef struct {
    size_t id;
} compilation_file_ref;

typedef struct {
    compilation_file_ref file;
    struct { size_t id; } token;
} compilation_token_ref;

typedef struct {
    compilation_file_ref file;
    struct { size_t id; } node;
} compilation_node_ref;

#define compilation_node_ref_delete(self) ((void) (self))

Slice_instantiate(compilation_node_ref);
Vector_instantiate(compilation_node_ref);

const compilation_file_t *compilation_file(const compilation_t *self, compilation_file_ref ref);

const Token *compilation_token(const compilation_t *self, compilation_token_ref ref);

const Node *compilation_node(const compilation_t *self, compilation_node_ref ref);

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
    Vector(Token) tokens;
    Vector(Node) nodes;
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
