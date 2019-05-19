#include <system.h>
#include <lib/stdio.h>
#include "symbols.h"

#include "types.h"
#include "env.h"

void SymbolScope_delete(SymbolScope *self)
{
    _Vector_delete(&self->t.nodes);
    _Vector_delete(&self->t.entries);
}

Symbols Symbols_new(Allocator *allocator, Types *types, Slice(SymbolInitializer) init, Slice(SymbolInitializer_intrin) initIntrin)
{
    Symbols ret = (Symbols) {
            .allocator = allocator,
            .scopes = Vector_new(allocator)
    };
    Symbols *self = &ret;
    Symbols_push(self);
    Slice_loop(&init, i) {
        SymbolInitializer it = *Slice_at(&init, i);
        Symbols_define(self, it.id, (Symbol) {
                .file = {0},
                .type = it.value.type,
                .value = it.value
        });
    }
    Slice_loop(&initIntrin, i) {
        SymbolInitializer_intrin it = *Slice_at(&initIntrin, i);
        TypeRef T = it.value->load(types);
        Symbols_define(self, it.id, (Symbol) {
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

static SymbolScope SymbolScope_new(Allocator *allocator, size_t parent)
{
    SymbolScope ret = (SymbolScope) {.parent = parent};
    SymbolScope *self = &ret;
    Trie_new(Symbol, allocator, &self->t);
    return ret;
}

static bool SymbolScope_get(SymbolScope *self, String ident, Symbol *out)
{
    return Trie_get((void *) &self->t, ident.bytes, out);
}

static void SymbolScope_set(SymbolScope *self, String ident, Symbol val)
{
    Trie_set((void *) &self->t, ident.bytes, &val, sizeof(TrieNode(Symbol)));
}

void Symbols_push(Symbols *symbols)
{
    Allocator *allocator = symbols->allocator;
    size_t size = Vector_size(&symbols->scopes);
    size_t parent = !size ? 0 : size - 1;
    SymbolScope newscope = SymbolScope_new(allocator, parent);
    Vector_push(&symbols->scopes, newscope);
}

void Symbols_pop(Symbols *symbols)
{
    SymbolScope_delete(Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1));
    Vector_pop(&symbols->scopes);
}

bool Symbols_lookup(const Symbols *symbols, String ident, Symbol *out)
{
    size_t i = Vector_size(&symbols->scopes) - 1;
    for (;;) {
        SymbolScope *it = Vector_at(&symbols->scopes, i);
        if (SymbolScope_get(it, ident, out)) {
            return true;
        }
        const size_t next = it->parent;
        if (i == next) {
            return false;
        }
        i = next;
    }
}

void Symbols_define(Symbols *symbols, String ident, Symbol sym)
{
    SymbolScope *top = Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1);
    SymbolScope_set(top, ident, sym);
}
