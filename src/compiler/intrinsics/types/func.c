#include <system.h>
#include "func.h"

#include "../_.h"
#include "../../phases/03-eval/eval.h"

static void types_func_args_types(Env env, nodelist iter, type_id out[]);

INTRINSIC_IMPL(types_func, ((type_id[]) {
        types->t_expr,
        types->t_unit,
}))
{
    const value_t *arg_args = Slice_at(&argv, 0);

    nodelist children = nodelist_iterator(env.compilation, arg_args->u.expr.value);
    const size_t argc = children._n;
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

static void types_func_args_types(Env env, nodelist iter, type_id out[])
{
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(env.compilation, ref);
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
