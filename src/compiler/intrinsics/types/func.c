#include <system.h>
#include "func.h"

#include "../_.h"
#include "../../phases/03-eval/eval.h"

static void types_func_args_types(Env env, Slice(node_t) args, type_id out[]);

INTRINSIC_IMPL(types_func, ((type_id[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &Slice_data(&argv)[0];

    const Slice(node_t) children = node_list_children(node_get(env.nodes, arg_args->u.expr.value));
    const size_t argc = Slice_size(&children);
    assert(argc >= 2 && "has enough arguments");
    type_id *Ts = realloc(NULL, sizeof(type_id) * argc);
    types_func_args_types(env, children, Ts);
    type_id T = type_func_new(env.types, Ts, argc);
    free(Ts);
    return (value_t) {
            .type = env.types->t_type,
            .node = self,
            .u.type.value = T,
    };
}

static void types_func_args_types(Env env, const Slice(node_t) args, type_id out[])
{
    size_t argc = Slice_size(&args);
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(&Slice_data(&args)[i], env.nodes);
        const value_t v = eval_node(env, it);
        type_id T = v.type;
        if (T.value == env.types->t_unit.value) {
            out[i] = env.types->t_unit;
            continue;
        }
        assert(T.value == env.types->t_type.value && "argument is a type");
        out[i] = v.u.type.value;
    }
}
