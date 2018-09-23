#include <system.h>
#include "func.h"

#include "../_.h"
#include "../../phases/03-eval/eval.h"

static void types_func_args_types(Env env, Slice(node_t) args, size_t argc, type_id out[]);

INTRINSIC_IMPL(types_func, ((type_id[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = &Slice_data(&argv)[0];

    const node_t *args = node_get(env.nodes, arg_args->u.expr.value);
    const Slice(node_t) children = node_list_children(args);
    const size_t argc = Slice_size(&children);
    assert(argc >= 2 && "has enough arguments");
    type_id *Ts = realloc(NULL, sizeof(type_id) * argc);
    types_func_args_types(env, children, argc, Ts);
    type_id T = type_func_new(env.types, Ts, argc);
    free(Ts);
    return (value_t) {
            .type = env.types->t_type,
            .u.type.value = T,
    };
}

static void types_func_args_types(Env env, const Slice(node_t) args, size_t argc, type_id out[])
{
    for (size_t i = 0; i < argc; ++i) {
        const node_t *it = node_deref(&Slice_data(&args)[i], env.nodes);
        const value_t type = eval_node(env, it);
        if (type.type.value == env.types->t_unit.value) {
            out[i] = env.types->t_unit;
            continue;
        }
        assert(type.type.value == env.types->t_type.value && "argument is a type");
        out[i] = type.u.type.value;
    }
}
