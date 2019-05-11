#pragma once

#include <lib/vector.h>

#include <parser/token.h>
#include <parser/node.h>

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

const token_t *compilation_token(const compilation_t *self, compilation_token_ref ref);

const node_t *compilation_node(const compilation_t *self, compilation_node_ref ref);
