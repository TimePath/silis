#include <system.h>
#include "func.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

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
            const Node *idNode = compilation_node(env.compilation, id);
            assert(idNode->kind == Node_Atom && "argument is a name");
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
        const Node *idNode = compilation_node(env.compilation, id);
        assert(idNode->kind == Node_Atom && "argument is a name");
        out[i] = idNode->u.Atom.value;
    }
}
