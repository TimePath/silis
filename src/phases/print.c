#include "../system.h"
#include "print.h"

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
    if (it->kind == NODE_INVALID) {
        return state;
    }
    if (it->kind == NODE_LIST_BEGIN) {
        print_indent(f, &state);
        ++state.depth;
        fprintf(f, "(");
        fprintf(f, " ; children(first: %zu, last: %zu, size: %zu)", it->u.list.begin, it->u.list.end, it->u.list.size);
        state.needTab = true;
        state.needLine = true;
    } else if (it->kind == NODE_LIST_END) {
        --state.depth;
        print_indent(f, &state);
        fprintf(f, ")");
        state.needTab = true;
        state.needLine = true;
    } else {
        print_indent(f, &state);
        switch (it->kind) {
            case NODE_REF:
                fprintf(f, "var_%zu", it->u.ref.value.val);
                break;
            case NODE_ATOM:
                fprintf(f, "`" STR_PRINTF "`", STR_PRINTF_PASS(it->u.atom.value));
                break;
            case NODE_INTEGRAL:
                fprintf(f, "%zu", it->u.integral.value);
                break;
            case NODE_STRING:
                fprintf(f, "\"" STR_PRINTF "\"", STR_PRINTF_PASS(it->u.string.value));
                break;
            case NODE_INVALID:
            case NODE_LIST_BEGIN:
            case NODE_LIST_END:
                assert(false);
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}
