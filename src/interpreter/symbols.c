#include <system.h>
#include <lib/stdio.h>
#include "symbols.h"

#include "types.h"
#include "env.h"

void sym_scope_t_delete(sym_scope_t *self)
{
    _Vector_delete(&self->t.nodes);
    _Vector_delete(&self->t.entries);
}

symbols_t symbols_new(Allocator *allocator, types_t *types, Slice(InitialSymbol) init, Slice(InitialSymbol_intrin) initIntrin)
{
    symbols_t ret = symbols_t_new(allocator);
    symbols_t *self = &ret;
    sym_push(self);
    Slice_loop(&init, i) {
        InitialSymbol it = *Slice_at(&init, i);
        sym_def(self, it.id, (sym_t) {
                .file = {0},
                .type = it.value.type,
                .value = it.value
        });
    }
    Slice_loop(&initIntrin, i) {
        InitialSymbol_intrin it = *Slice_at(&initIntrin, i);
        type_id T = it.value->load(types);
        sym_def(self, it.id, (sym_t) {
                .file = {0},
                .type = T,
                .value = {
                        .type = T,
                        .u.intrinsic.value = it.value,
                        .flags.abstract = it.flags.abstract,
                        .flags.intrinsic = true,
                },
        });
    }
    return ret;
}

static sym_scope_t sym_trie_new(Allocator *allocator, size_t parent)
{
    sym_scope_t self = (sym_scope_t) {.parent = parent};
    Trie_new(sym_t, allocator, &self.t);
    return self;
}

static bool sym_trie_get(sym_scope_t *self, String ident, sym_t *out)
{
    return Trie_get((void *) &self->t, ident.bytes, out);
}

static void sym_trie_set(sym_scope_t *self, String ident, sym_t val)
{
    Trie_set((void *) &self->t, ident.bytes, &val, sizeof(TrieNode(sym_t)));
}

void sym_push(symbols_t *symbols)
{
    Allocator *allocator = symbols->_allocator;
    size_t size = Vector_size(&symbols->scopes);
    size_t parent = !size ? 0 : size - 1;
    sym_scope_t newscope = sym_trie_new(allocator, parent);
    Vector_push(&symbols->scopes, newscope);
}

void sym_pop(symbols_t *symbols)
{
    sym_scope_t_delete(Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1));
    Vector_pop(&symbols->scopes);
}

bool sym_lookup(const symbols_t *symbols, String ident, sym_t *out)
{
    size_t i = Vector_size(&symbols->scopes) - 1;
    for (;;) {
        sym_scope_t *it = Vector_at(&symbols->scopes, i);
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
    sym_scope_t *top = Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1);
    sym_trie_set(top, ident, sym);
}