#include <system.h>
#include "value.h"

#include "env.h"
#include "symbols.h"

value_t value_from(Env env, const node_t *n)
{
    switch (n->kind) {
        case NODE_INVALID:
        case NODE_REF:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
            assert(false);
            break;
        case NODE_ATOM: {
            const String ident = n->u.atom.value;
            sym_t symbol;
            bool defined = sym_lookup(env.symbols, ident, &symbol);
            (void) defined;
            assert(defined && "symbol is defined");
            return symbol.value;
        }
        case NODE_INTEGRAL:
            return (value_t) {
                    .type = env.types->t_int,
                    .node = n,
                    .u.integral.value = n->u.integral.value,
            };
        case NODE_STRING:
            return (value_t) {
                    .type = env.types->t_string,
                    .node = n,
                    .u.string.value = n->u.string.value,
            };
    }
    return (value_t) {.type = {.value = 0}};
}
