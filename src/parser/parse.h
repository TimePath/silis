#pragma once

#include "token.h"
#include "node.h"

typedef struct {
    Allocator *allocator;
    const Vector(token_t) tokens;
} parse_input;

typedef struct {
    const Vector(node_t) nodes;
    /// id of root node
    const size_t root_id;
} parse_output;

parse_output do_parse(parse_input in);
