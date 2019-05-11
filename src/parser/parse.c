#include <system.h>
#include "parse.h"

typedef struct {
    Vector(token_t) tokens;
    Vector(node_t) nodes;
    Vector(node_t) stack;
} parse_ctx;

static void parse_yield(parse_ctx *ctx, node_t it)
{
    Vector_push(&ctx->nodes, it);
}

static size_t do_parse_rec(parse_ctx *ctx, const token_t *it);

parse_output do_parse(parse_input in)
{
    Allocator *allocator = in.allocator;
    parse_ctx ctx = (parse_ctx) {
            .tokens = in.tokens,
            .nodes = Vector_new(allocator),
            .stack = Vector_new(allocator),
    };
    do_parse_rec(&ctx, Vector_at(&ctx.tokens, 0));
    assert(Vector_size(&ctx.stack) == 1 && "stack contains result");
    node_t *it = Vector_at(&ctx.stack, 0);
    assert(it->kind == NODE_REF && "result is a reference");
    size_t root_id = it->u.ref.value;
    Vector_pop(&ctx.stack);
    return (parse_output) {.nodes = ctx.nodes, .root_id = root_id};
}

static node_t convert(const token_t *it);

// depth-first search
static size_t do_parse_rec(parse_ctx *ctx, const token_t *it)
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
            it += do_parse_rec(ctx, it);
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
        parse_yield(ctx, header);
        for (size_t i = 0; i < argc; ++i) {
            parse_yield(ctx, *Vector_at(&ctx->stack, argv_begin + i));
        }
        for (size_t i = 0; i < argc; ++i) {
            Vector_pop(&ctx->stack);
        }
        const node_t footer = (node_t) {
            .kind = NODE_LIST_END,
            .token = end,
            .u.list_end.begin = autoid,
        };
        parse_yield(ctx, footer);
    }
    const node_t ret = (node_t) {
            .kind = NODE_REF,
            .u.ref.value = autoid,
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
