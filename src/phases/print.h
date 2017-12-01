#pragma once

#include "../ctx.h"

#include <stdbool.h>

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
} print_state_t;

print_state_t print(FILE *f, print_state_t state, const node_t *it);
