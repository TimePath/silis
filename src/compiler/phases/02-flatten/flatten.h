#pragma once

#include <compiler/token.h>
#include <compiler/node.h>

typedef struct {
    compilation_file_ref file;
    const Vector(token_t) tokens;
} flatten_input;

typedef struct {
    /// ordered list of expressions to be evaluated at runtime
    /// starting from 1
    const Vector(node_t) nodes;
    const size_t entry;
} flatten_output;

flatten_output do_flatten(flatten_input in);
