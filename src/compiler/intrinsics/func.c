#include <system.h>
#include "func.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(func, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &argv[0];
    const value_t *arg_body = &argv[1];

    const node_t *args = node_get(env.nodes, arg_args->u.expr.value);
    Slice(node_t) children = node_list_children(args);
    const size_t argc = Slice_size(&children);
    assert(argc >= 2 && "has enough arguments");
    type_id *Ts = realloc(NULL, sizeof(type_id) * argc);
    func_args_types(env, children, Ts);
    type_id T = type_func_new(env.types, Ts, argc);
    free(Ts);
    return (value_t) {
            .type = T,
            .u.func.value = arg_body->u.expr.value,
            .u.func.arglist = arg_args->u.expr.value,
    };
}

void func_args_types(Env env, const Slice(node_t) args, type_id out[])
{
    size_t argc = Slice_size(&args);
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(&Slice_data(&args)[i], env.nodes);
        const Slice(node_t) children = node_list_children(it);
        const size_t n = Slice_size(&children);
        if (n == 0) {
            out[i] = env.types->t_unit;
        } else if (n <= 2) {
            const node_t *node_type = &Slice_data(&children)[0];
            const value_t type = eval_node(env, node_type);
            assert(type.type.value == env.types->t_type.value);
            out[i] = type.u.type.value;
            if (n == 2) {
                const node_t *node_id = &Slice_data(&children)[1];
                assert(node_id->kind == NODE_ATOM);
                (void) (node_id);
            }
        } else {
            assert(false);
        }
    }
}

void func_args_names(Env env, const Slice(node_t) args, String out[])
{
    size_t argc = Slice_size(&args);
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(&Slice_data(&args)[i], env.nodes);
        const Slice(node_t) children = node_list_children(it);
        const size_t n = Slice_size(&children);
        if (n != 2) {
            out[i] = STR("");
        } else {
            const node_t *node_id = &Slice_data(&children)[1];
            assert(node_id->kind == NODE_ATOM);
            out[i] = node_id->u.atom.value;
        }
    }
}

static void func_args_load(Env env, const node_t *arglist, const value_t *argv);

value_t func_call(Env env, value_t func, const value_t *argv)
{
    if (func.type.value <= env.types->end_intrinsics) {
        return func.u.intrinsic.value(env, argv);
    }
    sym_push(env.symbols, 0);
    const node_t *body = node_get(env.nodes, func.u.func.value);
    const node_t *arglist = node_get(env.nodes, func.u.func.arglist);
    func_args_load(env, arglist, argv);
    const value_t ret = eval_node(env, body);
    sym_pop(env.symbols);
    return ret;
}

static void func_args_load(Env env, const node_t *arglist, const value_t *argv)
{
    const Slice(node_t) args = node_list_children(arglist);
    const size_t argc = Slice_size(&args);
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(&Slice_data(&args)[i], env.nodes);
        const Slice(node_t) children = node_list_children(it);
        const size_t n = Slice_size(&children);
        if (n == 2) {
            const node_t *node_id = &Slice_data(&children)[1];
            assert(node_id->kind == NODE_ATOM);

            const value_t *v = &argv[i];
            sym_def(env.symbols, node_id->u.atom.value, (sym_t) {
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}
