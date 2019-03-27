#include <system.h>
#include "node.h"
#include "env.h"

#include <lib/stdio.h>

bool nodelist_next(nodelist *self, compilation_node_ref *out)
{
    if (self->_i < self->_n) {
        size_t i = self->_i++;
        if (out) {
            *out = nodelist_get(self, i);
        }
        return true;
    }
    return false;
}

compilation_node_ref nodelist_get(nodelist *self, size_t index)
{
    return (compilation_node_ref) {
            .file = self->head.file,
            .node = { .id = self->head.node.id + index }
    };
}

nodelist nodelist_iterator(const compilation_t *compilation, compilation_node_ref list)
{
    const node_t *node = compilation_node(compilation, list);
    assert(node->kind == NODE_LIST_BEGIN);
    return (nodelist) {
            .compilation = compilation,
            .head = (compilation_node_ref) { .file = list.file, .node = { .id = list.node.id + 1 }},
            ._i = 0,
            ._n = node->u.list.size,
    };
}

compilation_node_ref node_deref(const compilation_t *compilation, compilation_node_ref ref)
{
    const node_t *it = compilation_node(compilation, ref);
    if (it->kind == NODE_REF) {
        ref = it->u.ref.value;
        assert(compilation_node(compilation, ref)->kind == NODE_LIST_BEGIN && "references refer to lists");
    }
    return ref;
}

typedef struct {
    File *out;
    Slice(node_t) nodes;
} node_print_ctx_t;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    uint8_t _padding[6];
} node_print_state_t;

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it, size_t id);

void node_print(File *f, Slice(node_t) nodes)
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
        const node_t *it = &Slice_data(&nodes)[i];
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
    if (state->needLine) {
        fprintf_s(ctx->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(ctx->out, String_indent(2 * state->depth));
        state->needTab = false;
    }
}

static node_print_state_t _node_print(node_print_ctx_t *ctx, node_print_state_t state, const node_t *it, size_t id)
{
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
                assert(false);
                break;
            case NODE_REF:
                fprintf_s(ctx->out, STR("var_"));
                fprintf_zu(ctx->out, it->u.ref.value.node.id);
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
