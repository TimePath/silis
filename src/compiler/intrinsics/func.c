#include <system.h>
#include "func.h"

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(func, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = interpreter->allocator;
    const value_t *arg_args = Slice_at(&argv, 0);
    const value_t *arg_body = Slice_at(&argv, 1);

    nodelist children = nodelist_iterator(interpreter, arg_args->u.expr.value);
    const size_t argc = children._n;
    assert(argc >= 2 && "has enough arguments");
    TypeRef *Ts = malloc(sizeof(TypeRef) * argc);
    func_args_types(interpreter, children, Ts);
    TypeRef T = Types_register_func(interpreter->types, Slice_of_n(TypeRef, Ts, argc));
    free(Ts);
    return (value_t) {
            .type = T,
            .node = self,
            .u.func.value = arg_body->u.expr.value,
            .u.func.arglist = arg_args->u.expr.value,
    };
}

void func_args_types(Interpreter *interpreter, nodelist iter, TypeRef out[])
{
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(interpreter, ref);
        nodelist children = nodelist_iterator(interpreter, it);
        compilation_node_ref typeRef;
        if (!nodelist_next(&children, &typeRef)) {
            out[i] = interpreter->types->t_unit;
            continue;
        }
        const value_t type = eval_node(interpreter, typeRef);
        assert(type.type.value == interpreter->types->t_type.value && "argument is a type");
        out[i] = type.u.type.value;
        compilation_node_ref id;
        if (nodelist_next(&children, &id)) {
            const Node *idNode = compilation_node(interpreter, id);
            assert(idNode->kind == Node_Atom && "argument is a name");
            (void) (idNode);
        }
    }
}

void func_args_names(Interpreter *interpreter, nodelist iter, String out[])
{
    compilation_node_ref ref;
    for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
        compilation_node_ref it = node_deref(interpreter, ref);
        nodelist children = nodelist_iterator(interpreter, it);
        nodelist_next(&children, NULL);
        compilation_node_ref id;
        if (!nodelist_next(&children, &id)) {
            out[i] = STR("");
            continue;
        }
        const Node *idNode = compilation_node(interpreter, id);
        assert(idNode->kind == Node_Atom && "argument is a name");
        out[i] = idNode->u.Atom.value;
    }
}
