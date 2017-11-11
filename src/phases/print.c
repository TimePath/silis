#include "print.h"

#include <stdio.h>

static void print_indent(print_state_t *state) {
    if (state->needLine) {
        printf("\n");
        state->needLine = false;
    }
    if (state->needTab) {
        for (size_t i = 0; i < state->depth; ++i) printf("  ");
        state->needTab = false;
    }
}

print_state_t print(print_state_t state, const node_t *it) {
    if (it->type == NODE_INVALID) {
        return state;
    }
    if (it->type == NODE_LIST_BEGIN) {
        print_indent(&state);
        ++state.depth;
        printf("(");
        printf(" ; children(first: %lu, last: %lu, size: %lu)", it->u.list.begin, it->u.list.end, it->u.list.size);
        state.needTab = true;
        state.needLine = true;
    } else if (it->type == NODE_LIST_END) {
        --state.depth;
        print_indent(&state);
        printf(")");
        state.needTab = true;
        state.needLine = true;
    } else {
        print_indent(&state);
        switch (it->type) {
            case NODE_REF:
                printf("var_%lu", it->u.ref.value);
                break;
            default:
                printf("`" STR_PRINTF "`", STR_PRINTF_PASS(it->text));
                break;
            case NODE_INTEGRAL:
                printf("%lu", it->u.integral.value);
                break;
            case NODE_STRING:
                printf("\"" STR_PRINTF "\"", STR_PRINTF_PASS(it->u.string.value));
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}
