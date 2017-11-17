#pragma once

#include "../ctx.h"
#include "../lib/buffer.h"

size_t parse_list(ctx_t *ctx, buffer_t prog);

typedef enum {
    CHAR_INVALID,
    CHAR_WS,
    CHAR_SPECIAL,
    CHAR_SYM,
    CHAR_DIGIT,
    CHAR_ALPHA,
} char_rule_e;

extern char_rule_e parse_chars[];
