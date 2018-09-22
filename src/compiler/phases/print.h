#pragma once

#include "../node.h"

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    uint8_t padding[6];
} print_state_t;

print_state_t print(FILE *f, print_state_t state, const node_t *it);
