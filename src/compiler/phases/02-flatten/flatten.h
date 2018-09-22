#pragma once

#include "../../node.h"

typedef struct {
    const Vector(node_t) tokens;
} flatten_input;

typedef struct {
    /// ordered list of expressions to be evaluated at runtime
    /// starting from 1
    const Vector(node_t) nodes;
} flatten_output;

flatten_output do_flatten(flatten_input in);
