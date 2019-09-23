#include <prelude.h>
#include "func.h"

#include <lib/misc.h>

#include <interpreter/intrinsic.h>
#include <interpreter/eval.h>

INTRINSIC_IMPL(func, ((TypeRef[]) {
        types->t_expr, types->t_expr,
        types->t_unit,
}))
{
    Allocator *allocator = interpreter->allocator;
    const Value *arg_args = Slice_at(&argv, 0);
    const Value *arg_body = Slice_at(&argv, 1);

    NodeList children = NodeList_iterator(interpreter, arg_args->u.expr.value);
    const size_t argc = children._n;
    assert(argc >= 2 && "has enough arguments");
    TypeRef *Ts = new_arr(TypeRef, argc);
    func_args_types(interpreter, children, Ts);
    TypeRef T = Types_register_func(interpreter->types, Slice_of_n(TypeRef, Ts, argc));
    free(Ts);
    return (Value) {
            .type = T,
            .node = self,
            .u.func = {
                    .value = arg_body->u.expr.value,
                    .arglist = arg_args->u.expr.value,
            }
    };
}

void func_args_types(Interpreter *interpreter, NodeList iter, TypeRef out[])
{
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef it = Interpreter_lookup_node_ref(interpreter, ref);
        NodeList children = NodeList_iterator(interpreter, it);
        InterpreterFileNodeRef typeRef;
        if (!NodeList_next(&children, &typeRef)) {
            out[i] = interpreter->types->t_unit;
            continue;
        }
        const Value type = eval_node(interpreter, typeRef);
        assert(Ref_eq(type.type, interpreter->types->t_type) && "argument is a type");
        out[i] = type.u.type.value;
        InterpreterFileNodeRef id;
        if (NodeList_next(&children, &id)) {
            const Node *idNode = Interpreter_lookup_file_node(interpreter, id);
            assert(idNode->kind.val == Node_Atom && "argument is a name");
            (void) (idNode);
        }
    }
}

void func_args_names(Interpreter *interpreter, NodeList iter, String out[])
{
    InterpreterFileNodeRef ref;
    for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
        InterpreterFileNodeRef it = Interpreter_lookup_node_ref(interpreter, ref);
        NodeList children = NodeList_iterator(interpreter, it);
        NodeList_next(&children, NULL);
        InterpreterFileNodeRef id;
        if (!NodeList_next(&children, &id)) {
            out[i] = STR("");
            continue;
        }
        const Node *idNode = Interpreter_lookup_file_node(interpreter, id);
        assert(idNode->kind.val == Node_Atom && "argument is a name");
        out[i] = idNode->u.Atom.value;
    }
}
