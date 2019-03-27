#include <system.h>
#include "flatten.h"

typedef struct {
    compilation_file_ref file;
    Vector(token_t) tokens;
    Vector(node_t) nodes;
    Vector(node_t) stack;
} flatten_ctx_t;

static size_t do_flatten_rec(flatten_ctx_t *ctx, const token_t *it);

flatten_output do_flatten(flatten_input in)
{
    flatten_ctx_t ctx = (flatten_ctx_t) {
            .file = in.file,
            .tokens = in.tokens,
            .nodes = Vector_new(),
            .stack = Vector_new(),
    };
    Slice_loop(&Vector_toSlice(token_t, &ctx.tokens), i) {
        const token_t *it = &Vector_data(&ctx.tokens)[i];
        const size_t skip = do_flatten_rec(&ctx, it);
        Vector_pop(&ctx.stack); // ignore the final ref
        assert(Vector_size(&ctx.stack) == 0 && "stack is empty");
        i += skip;
    }
    const node_t *it = &Vector_data(&ctx.nodes)[Vector_size(&ctx.nodes) - 1];
    assert(it->kind == NODE_LIST_END);
    return (flatten_output) {.nodes = ctx.nodes, .entry = it->u.list_end.begin};
}

static node_t convert(const token_t *it);

// depth-first search
static size_t do_flatten_rec(flatten_ctx_t *ctx, const token_t *it)
{
    const token_t *begin = it;
    if (it->kind != TOKEN_LIST_BEGIN) {
        node_t n = convert(it);
        Vector_push(&ctx->stack, n);
        return 1;
    }
    const size_t argv_begin = Vector_size(&ctx->stack);
    {
        ++it; // skip begin
        while (it->kind != TOKEN_LIST_END) {
            it += do_flatten_rec(ctx, it);
        }
    }
    const token_t *end = it;
    // just parsed a full expression
    const size_t argc = Vector_size(&ctx->stack) - argv_begin;
    size_t autoid = Vector_size(&ctx->nodes) + 1;
    {
        const node_t header = (node_t) {
                .kind = NODE_LIST_BEGIN,
                .token = begin,
                .u.list.begin = !argc ? 0 : autoid + 1,
                .u.list.end = !argc ? 0 : autoid + argc,
                .u.list.size = argc,
        };
        Vector_push(&ctx->nodes, header);
        for (size_t i = 0; i < argc; ++i) {
            Vector_push(&ctx->nodes, Vector_data(&ctx->stack)[argv_begin + i]);
        }
        for (size_t i = 0; i < argc; ++i) {
            Vector_pop(&ctx->stack);
        }
        const node_t footer = (node_t) {
            .kind = NODE_LIST_END,
            .token = end,
            .u.list_end.begin = autoid,
        };
        Vector_push(&ctx->nodes, footer);
    }
    const node_t ret = (node_t) {
            .kind = NODE_REF,
            .u.ref.value = (compilation_node_ref) {.file = ctx->file, .node = {.id = autoid}},
    };
    Vector_push(&ctx->stack, ret);
    return 1 + (size_t) (end - begin);
}

static node_t convert(const token_t *it)
{
    switch (it->kind) {
        case TOKEN_INVALID:
        case TOKEN_LIST_BEGIN:
        case TOKEN_LIST_END:
            assert(false);
            break;
        case TOKEN_ATOM:
            return (node_t) {
                    .kind = NODE_ATOM,
                    .token = it,
                    .u.atom = {
                            .value = it->u.atom.value,
                    },
            };
        case TOKEN_INTEGRAL:
            return (node_t) {
                    .kind = NODE_INTEGRAL,
                    .token = it,
                    .u.integral = {
                            .value = it->u.integral.value,
                    },
            };
        case TOKEN_STRING:
            return (node_t) {
                    .kind = NODE_STRING,
                    .token = it,
                    .u.string = {
                            .value = it->u.string.value,
                    },
            };
    }
    return (node_t) {
            .kind = NODE_INVALID,
            .token = it,
    };
}
