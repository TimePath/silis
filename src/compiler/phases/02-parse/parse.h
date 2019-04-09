#pragma once

#include <compiler/token.h>
#include <compiler/node.h>

typedef struct {
    compilation_file_ref file;
    const Vector(token_t) tokens;
} parse_input;

typedef struct {
    /// ordered list of expressions to be evaluated at runtime
    /// starting from 1
    const Vector(node_t) nodes;
    const size_t root;
} parse_output;

parse_output do_parse(parse_input in);
