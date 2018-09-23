#include <system.h>
#include "symbols.h"

#include "types.h"
#include "env.h"

symbols_t symbols_new(types_t *types, Slice(InitialSymbol) init)
{
    symbols_t ret = symbols_t_new();
    symbols_t *self = &ret;

    sym_push(self, 0);
    sym_def(self, STR("#types/string"), (sym_t) {
            .type = types->t_type,
            .value = {
                    .type = types->t_type,
                    .u.type.value = types->t_string,
                    .flags.intrinsic = true,
            },
    });
    sym_def(self, STR("#types/int"), (sym_t) {
            .type = types->t_type,
            .value = {
                    .type = types->t_type,
                    .u.type.value = types->t_int,
                    .flags.intrinsic = true,
            },
    });

    Slice_loop(&init, i) {
        InitialSymbol it = Slice_data(&init)[i];
        type_id T = it.value->load(types);
        sym_def(self, it.id, (sym_t) {
                .type = T,
                .value = {
                        .type = T,
                        .u.intrinsic.value = it.value,
                        .flags.intrinsic = true,
                },
        });
    }
    return ret;
}

static sym_scope_t sym_trie_new(size_t parent)
{
    sym_scope_t self = (sym_scope_t) {.parent = parent};
    Trie_new(sym_t, &self.t);
    return self;
}

static void sym_trie_delete(sym_scope_t *self)
{
    Vector_delete(&self->t.nodes);
    Vector_delete(&self->t.entries);
}

static bool sym_trie_get(sym_scope_t *self, String ident, sym_t *out)
{
    return Trie_get((void *) &self->t, ident.bytes, out);
}

static void sym_trie_set(sym_scope_t *self, String ident, sym_t val)
{
    Trie_set((void *) &self->t, ident.bytes, &val, sizeof(TrieNode(sym_t)));
}

void sym_push(symbols_t *symbols, size_t parent)
{
    sym_scope_t newscope = sym_trie_new(parent);
    Vector_push(&symbols->scopes, newscope);
}

void sym_pop(symbols_t *symbols)
{
    sym_trie_delete(&Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1]);
    Vector_pop(&symbols->scopes);
}

bool sym_lookup(const symbols_t *symbols, String ident, sym_t *out)
{
    size_t i = Vector_size(&symbols->scopes) - 1;
    for (;;) {
        sym_scope_t *it = &Vector_data(&symbols->scopes)[i];
        if (sym_trie_get(it, ident, out)) {
            return true;
        }
        const size_t next = it->parent;
        if (i == next) {
            return false;
        }
        i = next;
    }
}

void sym_def(symbols_t *symbols, String ident, sym_t sym)
{
    sym_scope_t *top = &Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1];
    sym_trie_set(top, ident, sym);
}
