#include <prelude.h>
#include "value.h"

#include <lib/misc.h>

#include "interpreter.h"
#include "symbols.h"

Value Value_from(Interpreter *interpreter, InterpreterFileNodeRef it)
{
    const Node *n = Interpreter_lookup_file_node(interpreter, it);
    switch (n->kind.val) {
        case Node_INVALID:
        case Node_Ref:
        case Node_ListBegin:
        case Node_ListEnd:
            unreachable(break);
        case Node_Atom: {
            const String ident = n->u.Atom.value;
            Symbol symbol;
            bool defined = Symbols_lookup(interpreter->symbols, ident, &symbol);
            (void) defined;
            assert(defined && "symbol is defined");
            return symbol.value;
        }
        case Node_Integral:
            return (Value) {
                    .type = interpreter->types->t_int,
                    .node = it,
                    .u.integral.value = n->u.Integral.value,
            };
        case Node_String:
            return (Value) {
                    .type = interpreter->types->t_string,
                    .node = it,
                    .u.string.value = n->u.String.value,
            };
    }
    return (Value) {.type = {.value = 0}};
}
