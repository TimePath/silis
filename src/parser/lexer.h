#pragma once

#include <lib/result.h>
#include <lib/string.h>
#include <lib/vector.h>

#include "error.h"
#include "token.h"

Result_instantiate(Vector(Token), ParserError);

typedef struct {
    Allocator *allocator;
    const String source;
} silis_parser_lex_input;

Result(Vector(Token), ParserError) silis_parser_lex(silis_parser_lex_input in);
