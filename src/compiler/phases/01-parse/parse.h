#pragma once

#include <lib/string.h>

#include "../../ctx.h"

size_t parse_list(ctx_t *ctx, String prog);

typedef enum {
    CHAR_INVALID,
    CHAR_WS,
    CHAR_SPECIAL,
    CHAR_SYM,
    CHAR_DIGIT,
    CHAR_ALPHA,
} char_rule_e;

char_rule_e parse_char(size_t c);
