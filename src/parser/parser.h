#pragma once

#include <lib/vector.h>

#include "token.h"
#include "node.h"

typedef struct {
    Slice(Token) tokens;
    Allocator *allocator;
} silis_parser_parse_input;

typedef struct {
    const Vector(Node) nodes;
    /// id of root node
    const Ref(Node) root;
} silis_parser_parse_output;

silis_parser_parse_output silis_parser_parse(silis_parser_parse_input in);
