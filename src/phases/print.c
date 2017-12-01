#include "print.h"

#include <assert.h>

static void print_indent(FILE *f, print_state_t *state) {
    if (state->needLine) {
        fprintf(f, "\n");
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf(f, STR_PRINTF, STR_PRINTF_PASS(str_indent(2 * state->depth)));
        state->needTab = false;
    }
}

print_state_t print(FILE *f, print_state_t state, const node_t *it) {
    if (it->type == NODE_INVALID) {
        return state;
    }
    if (it->type == NODE_LIST_BEGIN) {
        print_indent(f, &state);
        ++state.depth;
        fprintf(f, "(");
        fprintf(f, " ; children(first: %lu, last: %lu, size: %lu)", it->u.list.begin, it->u.list.end, it->u.list.size);
        state.needTab = true;
        state.needLine = true;
    } else if (it->type == NODE_LIST_END) {
        --state.depth;
        print_indent(f, &state);
        fprintf(f, ")");
        state.needTab = true;
        state.needLine = true;
    } else {
        print_indent(f, &state);
        switch (it->type) {
            default:
                assert(false);
                break;
            case NODE_REF:
                fprintf(f, "var_%lu", it->u.ref.value.val);
                break;
            case NODE_ATOM:
                fprintf(f, "`" STR_PRINTF "`", STR_PRINTF_PASS(it->u.atom.value));
                break;
            case NODE_INTEGRAL:
                fprintf(f, "%lu", it->u.integral.value);
                break;
            case NODE_STRING:
                fprintf(f, "\"" STR_PRINTF "\"", STR_PRINTF_PASS(it->u.string.value));
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}
