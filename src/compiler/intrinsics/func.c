#include <system.h>
#include "func.h"

#include "_.h"
#include <compiler/phases/03-eval/eval.h>

INTRINSIC_IMPL(func, ((type_id[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = env.allocator;
    const value_t *arg_args = Slice_at(&argv, 0);
    const value_t *arg_body = Slice_at(&argv, 1);

    nodelist children = nodelist_iterator(env.compilation, arg_args->u.expr.value);
    const size_t argc = children._n;
    assert(argc >= 2 && "has enough arguments");
    type_id *Ts = malloc(sizeof(type_id) * argc);
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

void func_args_types(Env env, nodelist iter, type_id out[])
{
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        nodelist children = nodelist_iterator(env.compilation, it);
        compilation_node_ref typeRef;
        if (!nodelist_next(&children, &typeRef)) {
            out[i] = env.types->t_unit;
            continue;
        }
        const value_t type = eval_node(env, typeRef);
        assert(type.type.value == env.types->t_type.value && "argument is a type");
        out[i] = type.u.type.value;
        compilation_node_ref id;
        if (nodelist_next(&children, &id)) {
            const node_t *idNode = compilation_node(env.compilation, id);
            assert(idNode->kind == NODE_ATOM && "argument is a name");
            (void) (idNode);
        }
    }
}

void func_args_names(Env env, nodelist iter, String out[])
{
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        nodelist children = nodelist_iterator(env.compilation, it);
        nodelist_next(&children, NULL);
        compilation_node_ref id;
        if (!nodelist_next(&children, &id)) {
            out[i] = STR("");
            continue;
        }
        const node_t *idNode = compilation_node(env.compilation, id);
        assert(idNode->kind == NODE_ATOM && "argument is a name");
        out[i] = idNode->u.atom.value;
    }
}

static void func_args_load(Env env, compilation_node_ref arglist, const Slice(value_t) argv);

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
    nodelist iter = nodelist_iterator(env.compilation, arglist);
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
        nodelist children = nodelist_iterator(env.compilation, it);
        nodelist_next(&children, NULL);
        compilation_node_ref id;
        if (nodelist_next(&children, &id)) {
            const node_t *idNode = compilation_node(env.compilation, id);
            assert(idNode->kind == NODE_ATOM && "argument is a name");

            const value_t *v = Slice_at(&argv, i);
            sym_def(env.symbols, idNode->u.atom.value, (sym_t) {
                    .file = {0},
                    .type = v->type,
                    .value = *v,
                    .flags.eval = true,
            });
        }
    }
}
