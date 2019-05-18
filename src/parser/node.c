#include <system.h>
#include "node.h"

#include <lib/stdio.h>

typedef struct {
    Allocator *allocator;
    File *out;
    Slice(node_t) nodes;
} node_print_ctx_t;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    PADDING(6)
} node_print_state_t;

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it, size_t id);

void node_print(Allocator *allocator, File *f, Slice(node_t) nodes)
{
    node_print_ctx_t ctx = {
            .allocator = allocator,
            .out = f,
            .nodes = nodes,
    };
    node_print_state_t state = (node_print_state_t) {
            .depth = 0,
            .needLine = false,
            .needTab = false,
    };
    Slice_loop(&nodes, i) {
        const node_t *it = Slice_at(&nodes, i);
        state = _node_print(&ctx, state, it, i + 1);
        if (it->kind == NODE_LIST_END) {
            fprintf_s(f, STR("\n"));
            state = (node_print_state_t) {
                    .depth = 0,
                    .needLine = false,
                    .needTab = false,
            };
        }
    }
}

static void _node_print_indent(node_print_ctx_t *ctx, node_print_state_t *state)
{
    Allocator *allocator = ctx->allocator;
    if (state->needLine) {
        fprintf_s(ctx->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(ctx->out, String_indent(allocator, 2 * state->depth));
        state->needTab = false;
    }
}

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it, size_t id)
{
    if (it->kind == NODE_INVALID) {
        unreachable();
        return state;
    }
    if (it->kind == NODE_LIST_BEGIN) {
        _node_print_indent(ctx, &state);
        ++state.depth;
        fprintf_s(ctx->out, STR("("));
        fprintf_s(ctx->out, STR(" ;"));
        {
            {
                fprintf_s(ctx->out, STR(" .id = "));
                fprintf_zu(ctx->out, id);
                fprintf_s(ctx->out, STR(","));
            }
            {
                fprintf_s(ctx->out, STR(" .children = {"));
                {
                    fprintf_s(ctx->out, STR(" .begin = "));
                    fprintf_zu(ctx->out, it->u.list.begin);
                    fprintf_s(ctx->out, STR(","));
                }
                {
                    fprintf_s(ctx->out, STR(" .end = "));
                    fprintf_zu(ctx->out, it->u.list.end);
                    fprintf_s(ctx->out, STR(","));
                }
                {
                    fprintf_s(ctx->out, STR(" .size = "));
                    fprintf_zu(ctx->out, it->u.list.size);
                    fprintf_s(ctx->out, STR(","));
                }
                fprintf_s(ctx->out, STR(" },"));
            }
        }
        state.needTab = true;
        state.needLine = true;
    } else if (it->kind == NODE_LIST_END) {
        --state.depth;
        _node_print_indent(ctx, &state);
        state.needTab = true;
        state.needLine = true;
        fprintf_s(ctx->out, STR(")"));
        fprintf_s(ctx->out, STR(" ;"));
        {
            {
                fprintf_s(ctx->out, STR(" .id = "));
                fprintf_zu(ctx->out, id);
                fprintf_s(ctx->out, STR(","));
            }
            {
                fprintf_s(ctx->out, STR(" .begin = "));
                fprintf_zu(ctx->out, it->u.list_end.begin);
                fprintf_s(ctx->out, STR(","));
            }
        }
    } else {
        _node_print_indent(ctx, &state);
        switch (it->kind) {
            case NODE_INVALID:
            case NODE_LIST_BEGIN:
            case NODE_LIST_END:
                unreachable();
                break;
            case NODE_REF:
                fprintf_s(ctx->out, STR("var_"));
                fprintf_zu(ctx->out, it->u.ref.value);
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case NODE_ATOM:
                fprintf_s(ctx->out, STR("`"));
                fprintf_s(ctx->out, it->u.atom.value);
                fprintf_s(ctx->out, STR("`"));
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case NODE_INTEGRAL:
                fprintf_zu(ctx->out, it->u.integral.value);
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case NODE_STRING:
                fprintf_s(ctx->out, STR("\""));
                fprintf_s(ctx->out, it->u.string.value);
                fprintf_s(ctx->out, STR("\""));
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}
