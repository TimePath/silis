#include <system.h>
#include "flatten.h"

static size_t do_flatten_rec(ctx_t *ctx, Vector(node_t) *stack, const node_t *begin);

void do_flatten(ctx_t *ctx)
{
    {
        // make usable ids start from 1
        Vector(node_t) *out = &ctx->flatten.out;
        const node_t dummy = (node_t) {.kind = NODE_INVALID};
        Vector_push(out, dummy);
        assert(Vector_size(out) == 1);
    }

    const Vector(node_t) *read = &ctx->parse.out;
    const node_t *begin = &Vector_data(read)[0];
    const node_t *end = &Vector_data(read)[Vector_size(read) - 1];
    Vector(node_t) stack = {0};
    for (const node_t *it = begin; it < end;) {
        const size_t skip = do_flatten_rec(ctx, &stack, it);
        Vector_pop(&stack); // ignore the final ref
        assert(Vector_size(&stack) == 0);
        it += skip;
    }
}

static size_t do_flatten_rec(ctx_t *ctx, Vector(node_t) *stack, const node_t *begin)
{
    const node_t *it = begin;
    if (it->kind != NODE_LIST_BEGIN) {
        Vector_push(stack, *it);
        return 1;
    }
    const size_t argv_begin = Vector_size(stack);
    // depth-first flatten
    {
        ++it; // skip begin
        while (it->kind != NODE_LIST_END) {
            it += do_flatten_rec(ctx, stack, it);
        }
        ++it; // skip end
    }
    Vector(node_t) *out = &ctx->flatten.out;
    const size_t argv_end = Vector_size(stack) - 1;
    const size_t argc = argv_end - argv_begin + 1;
    const node_id refIdx = (node_id) {Vector_size(out)};
    // just parsed a full expression
    {
        const node_t open = (node_t) {
                .kind = NODE_LIST_BEGIN,
                .u.list.begin = !argc ? 0 : Vector_size(out) + 1,
                .u.list.end = !argc ? 0 : Vector_size(out) + argc,
                .u.list.size = argc,
        };
        Vector_push(out, open);
        for (size_t i = 0; i < argc; ++i) {
            Vector_push(out, Vector_data(stack)[argv_begin + i]);
        }
        for (size_t i = 0; i < argc; ++i) {
            Vector_pop(stack);
        }
        const node_t close = (node_t) {.kind = NODE_LIST_END};
        Vector_push(out, close);
    }
    const node_t ret = (node_t) {
            .kind = NODE_REF,
            .u.ref.value = refIdx,
    };
    Vector_push(stack, ret);
    return (size_t) (it - begin);
}
