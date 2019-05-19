#include <system.h>
#include "value.h"

#include "env.h"
#include "symbols.h"

value_t value_from(Env env, compilation_node_ref it)
{
    const Node *n = compilation_node(env.compilation, it);
    switch (n->kind) {
        case Node_INVALID:
        case Node_Ref:
        case Node_ListBegin:
        case Node_ListEnd:
            unreachable();
            break;
        case Node_Atom: {
            const String ident = n->u.Atom.value;
            sym_t symbol;
            bool defined = sym_lookup(env.symbols, ident, &symbol);
            (void) defined;
            assert(defined && "symbol is defined");
            return symbol.value;
        }
        case Node_Integral:
            return (value_t) {
                    .type = env.types->t_int,
                    .node = it,
                    .u.integral.value = n->u.Integral.value,
            };
        case Node_String:
            return (value_t) {
                    .type = env.types->t_string,
                    .node = it,
                    .u.string.value = n->u.String.value,
            };
    }
    return (value_t) {.type = {.value = 0}};
}
