#include <system.h>
#include "func.h"

#include "_.h"
#include "../phases/03-eval/eval.h"

INTRINSIC_IMPL(func, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &Slice_data(&argv)[0];
    const value_t *arg_body = &Slice_data(&argv)[1];

    Slice(node_t) children = node_list_children(env.compilation, arg_args->u.expr.value);
    const size_t argc = Slice_size(&children);
    assert(argc >= 2 && "has enough arguments");
    type_id *Ts = realloc(NULL, sizeof(type_id) * argc);
    func_args_types(env, children, Ts);
    type_id T = type_func_new(env.types, Ts, argc);
    free(Ts);
    return (value_t) {
            .type = T,
            .node = self,
            .u.func.value = arg_body->u.expr.value,
            .u.func.arglist = arg_args->u.expr.value,
    };
}

void func_args_types(Env env, const Slice(node_t) args, type_id out[])
{
    nodelist iter = nodelist_iterator(args, env.compilation);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        const Slice(node_t) children = node_list_children(env.compilation, it);
        const size_t n = Slice_size(&children);
        if (n == 0) {
            out[i] = env.types->t_unit;
        } else if (n <= 2) {
            const node_t *node_type = &Slice_data(&children)[0];
            const value_t type = eval_node(env, compilation_node_find(env.compilation, node_type));
            assert(type.type.value == env.types->t_type.value && "argument is a type");
            out[i] = type.u.type.value;
            if (n == 2) {
                const node_t *node_id = &Slice_data(&children)[1];
                assert(node_id->kind == NODE_ATOM && "argument is a name");
                (void) (node_id);
            }
        } else {
            assert(false);
        }
    }
}

void func_args_names(Env env, const Slice(node_t) args, String out[])
{
    nodelist iter = nodelist_iterator(args, env.compilation);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        const Slice(node_t) children = node_list_children(env.compilation, it);
        const size_t n = Slice_size(&children);
        if (n != 2) {
            out[i] = STR("");
        } else {
            const node_t *node_id = &Slice_data(&children)[1];
            assert(node_id->kind == NODE_ATOM && "argument is a name");
            out[i] = node_id->u.atom.value;
        }
    }
}

static void func_args_load(Env env, compilation_node_ref arglist, Slice(value_t) argv);

value_t func_call(Env env, value_t func, const Slice(value_t) argv, compilation_node_ref self)
{
    if (func.flags.intrinsic) {
        return func.u.intrinsic.value->call(env, self, argv);
    }
    sym_push(env.symbols);
    compilation_node_ref body = func.u.func.value;
    compilation_node_ref arglist = func.u.func.arglist;
    func_args_load(env, arglist, argv);
    const value_t ret = eval_node(env, body);
    sym_pop(env.symbols);
    return ret;
}

static void func_args_load(Env env, compilation_node_ref arglist, const Slice(value_t) argv)
{
    nodelist iter = nodelist_iterator(node_list_children(env.compilation, arglist), env.compilation);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        const Slice(node_t) children = node_list_children(env.compilation, it);
        const size_t n = Slice_size(&children);
        if (n == 2) {
            const node_t *node_id = &Slice_data(&children)[1];
            assert(node_id->kind == NODE_ATOM && "argument is a name");

            const value_t *v = &Slice_data(&argv)[i];
            sym_def(env.symbols, node_id->u.atom.value, (sym_t) {
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}
