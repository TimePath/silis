#include <system.h>

#include "print.h"

#include <lib/stdio.h>

static void print_indent(FILE *f, print_state_t *state) {
    if (state->needLine) {
        fprintf_s(f, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(f, String_indent(2 * state->depth));
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
        fprintf_s(f, STR("("));
        fprintf_s(f, STR(" ; children(first: "));
        fprintf_zu(f, it->u.list.begin);
        fprintf_s(f, STR(", last: "));
        fprintf_zu(f, it->u.list.end);
        fprintf_s(f, STR(", size: "));
        fprintf_zu(f, it->u.list.size);
        fprintf_s(f, STR(")"));
        state.needTab = true;
        state.needLine = true;
    } else if (it->kind == NODE_LIST_END) {
        --state.depth;
        print_indent(f, &state);
        fprintf_s(f, STR(")"));
        state.needTab = true;
        state.needLine = true;
    } else {
        print_indent(f, &state);
        switch (it->kind) {
            case NODE_REF:
                fprintf_s(f, STR("var_"));
                fprintf_zu(f, it->u.ref.value.val);
                break;
            case NODE_ATOM:
                fprintf_s(f, STR("`"));
                fprintf_s(f, it->u.atom.value);
                fprintf_s(f, STR("`"));
                break;
            case NODE_INTEGRAL:
                fprintf_zu(f, it->u.integral.value);
                break;
            case NODE_STRING:
                fprintf_s(f, STR("\""));
                fprintf_s(f, it->u.string.value);
                fprintf_s(f, STR("\""));
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
