#pragma once

#include <lib/result.h>
#include <lib/string.h>

#include "error.h"
#include "token.h"

typedef struct {
    Allocator *allocator;
    const String source;
} lex_input;

Result_instantiate(Vector(token_t), ParserError);

Result(Vector(token_t), ParserError) silis_parser_lex(lex_input in);
