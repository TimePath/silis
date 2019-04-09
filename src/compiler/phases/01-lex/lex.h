#pragma once

#include <lib/string.h>

#include <compiler/token.h>

typedef struct {
    const String source;
} lex_input;

typedef struct {
    const Vector(token_t) tokens;
} lex_output;

lex_output do_lex(lex_input in);
