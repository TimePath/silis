#include "flatten.h"

#include <assert.h>

static size_t do_flatten_rec(ctx_t *ctx, vec_t(node_t) *stack, const node_t *begin);

void do_flatten(ctx_t *ctx) {
    {
        // make usable ids start from 1
        vec_t(node_t) *out = &ctx->flatten.out;
        const node_t dummy = (node_t) {.type = NODE_INVALID};
        vec_push(out, dummy);
        assert(out->size == 1);
    }

    const vec_t(node_t) *read = &ctx->parse.out;
    const node_t *begin = &read->data[0];
    const node_t *end = &read->data[read->size - 1];
    vec_t(node_t) stack = {0};
    for (const node_t *it = begin; it < end;) {
        const size_t skip = do_flatten_rec(ctx, &stack, it);
        vec_pop(&stack); // ignore the final ref
        assert(stack.size == 0);
        it += skip;
    }
}

static size_t do_flatten_rec(ctx_t *ctx, vec_t(node_t) *stack, const node_t *begin) {
    const node_t *it = begin;
    if (it->type != NODE_LIST_BEGIN) {
        vec_push(stack, *it);
        return 1;
    }
    const size_t argv_begin = stack->size;
    // depth-first flatten
    {
        ++it; // skip begin
        while (it->type != NODE_LIST_END) {
            it += do_flatten_rec(ctx, stack, it);
        }
        ++it; // skip end
    }
    vec_t(node_t) *out = &ctx->flatten.out;
    const size_t argv_end = stack->size - 1;
    const size_t argc = argv_end - argv_begin + 1;
    const node_ref_t refIdx = (node_ref_t) {out->size};
    // just parsed a full expression
    {
        const node_t open = (node_t) {
                .type = NODE_LIST_BEGIN,
                .u.list.begin = !argc ? 0 : out->size + 1,
                .u.list.end = !argc ? 0 : out->size + argc,
                .u.list.size = argc,
        };
        vec_push(out, open);
        for (size_t i = 0; i < argc; ++i) {
            vec_push(out, stack->data[argv_begin + i]);
        }
        for (size_t i = 0; i < argc; ++i) {
            vec_pop(stack);
        }
        const node_t close = (node_t) {.type = NODE_LIST_END};
        vec_push(out, close);
    }
    const node_t ret = (node_t) {
            .type = NODE_REF,
            .u.ref.value = refIdx,
    };
    vec_push(stack, ret);
    return it - begin;
}
