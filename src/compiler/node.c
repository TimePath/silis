#include <system.h>
#include "node.h"
#include "env.h"

#include <lib/stdio.h>

Slice(node_t) node_list_children(const node_t *list)
{
    assert(list->kind == NODE_LIST_BEGIN);
    size_t n = list->u.list.size;
    const node_t *begin = list + 1;
    return (Slice(node_t)) {._begin = begin, ._end = begin + n};
}

const node_t *node_get(const Vector(node_t) *nodes, node_id ref)
{
    return &Vector_data(nodes)[ref.val];
}

node_id node_ref(const node_t *it, const Vector(node_t) *nodes)
{
    assert(it >= Vector_data(nodes) &&
           it <= &Vector_data(nodes)[Vector_size(nodes) - 1]);
    return (node_id) {.val = (size_t) (it - Vector_data(nodes))};
}

const node_t *node_deref(const node_t *it, const Vector(node_t) *nodes)
{
    if (it->kind != NODE_REF) {
        return it;
    }
    const node_t *ret = node_get(nodes, it->u.ref.value);
    assert(ret->kind == NODE_LIST_BEGIN && "references refer to lists");
    return ret;
}

typedef struct {
    FILE *out;
    Slice(node_t) nodes;
} node_print_ctx_t;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    uint8_t padding[6];
} node_print_state_t;

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it);

void node_print(FILE *f, Slice(node_t) nodes)
{
    node_print_ctx_t ctx = {
            .out = f,
            .nodes = nodes,
    };
    node_print_state_t state = (node_print_state_t) {
            .depth = 0,
            .needLine = false,
            .needTab = false,
    };
    Slice_loop(&nodes, i) {
        if (i < 1) { continue; }
        const node_t *it = &Slice_data(&nodes)[i];
        state = _node_print(&ctx, state, it);
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
    if (state->needLine) {
        fprintf_s(ctx->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(ctx->out, String_indent(2 * state->depth));
        state->needTab = false;
    }
}

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it)
{
    size_t id = (size_t) (it - ctx->nodes._begin);
    if (it->kind == NODE_INVALID) {
        assert(false);
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
        }
    } else {
        _node_print_indent(ctx, &state);
        switch (it->kind) {
            case NODE_INVALID:
            case NODE_LIST_BEGIN:
            case NODE_LIST_END:
                assert(false);
                break;
            case NODE_REF:
                fprintf_s(ctx->out, STR("var_"));
                fprintf_zu(ctx->out, it->u.ref.value.val);
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
