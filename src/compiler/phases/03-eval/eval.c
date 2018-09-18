#include <system.h>
#include "eval.h"

#include "../../intrinsics/func.h"

void do_eval(ctx_t *ctx)
{
    const node_t *it = node_get(ctx, (node_id) {Vector_size(&ctx->flatten.out) - 1});
    assert(it->kind == NODE_LIST_END);
    while ((--it)->kind != NODE_LIST_BEGIN) {}
    eval_list_block(ctx, it);
}

value_t eval_list_block(ctx_t *ctx, const node_t *it)
{
    assert(it->kind == NODE_LIST_BEGIN);
    const size_t n = it->u.list.size;
    const node_t *children = node_list_children(it);
    value_t ret = (value_t) {.type = ctx->state.types.t_unit};
    for (size_t i = 0; i < n; ++i) {
        ret = eval_node(ctx, &children[i]);
    }
    return ret;
}

value_t eval_node(ctx_t *ctx, const node_t *it)
{
    it = node_deref(ctx, it);
    if (it->kind != NODE_LIST_BEGIN) {
        return value_from(ctx, it);
    }
    const size_t n = it->u.list.size;
    if (!n) {
        return (value_t) {.type = ctx->state.types.t_unit};
    }
    const node_t *children = node_list_children(it);
    if (n == 1) {
        return eval_node(ctx, &children[0]);
    }
    const value_t func = eval_node(ctx, &children[0]);
    const type_t *T = type_lookup(ctx, func.type);
    assert(T->kind == TYPE_FUNCTION);

    const size_t ofs = Vector_size(&ctx->eval.stack);
    const type_id expr_t = ctx->state.types.t_expr;
    size_t T_argc = 0;
    // holds return value after this loop
    const type_t *link = T;
    for (; link->kind == TYPE_FUNCTION; ++T_argc) {
        assert((n - 1) > T_argc && "argument underflow");
        const node_t *arg = &children[T_argc + 1];
        const type_id arg_t = link->u.func.in;
        if (arg_t.value == expr_t.value) {
            value_t v = (value_t) {
                    .type = expr_t,
                    .u.expr.value = node_ref(ctx, node_deref(ctx, arg)),
            };
            Vector_push(&ctx->eval.stack, v);
        } else {
            value_t v = eval_node(ctx, arg);
            assert(v.type.value == arg_t.value);
            Vector_push(&ctx->eval.stack, v);
        }
        link = type_lookup(ctx, link->u.func.out);
    }
    assert((n - 1) == T_argc && "argument overflow");

    const value_t *argv = &Vector_data(&ctx->eval.stack)[ofs];
    value_t ret = func_call(ctx, func, argv);
    for (size_t i = 1; i < n; ++i) {
        Vector_pop(&ctx->eval.stack);
    }
    return ret;
}
