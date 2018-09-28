#pragma once

#include "node.h"

typedef struct {
    struct types_s *types;
    struct symbols_s *symbols;
    const Vector(node_t) *nodes;
    FILE *stdout;
} Env;
