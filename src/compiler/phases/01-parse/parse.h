#pragma once

#include <lib/string.h>

#include "../../node.h"

typedef struct {
    const String source;
} parse_input;

typedef struct {
    const Vector(node_t) tokens;
} parse_output;

parse_output do_parse(parse_input in);
