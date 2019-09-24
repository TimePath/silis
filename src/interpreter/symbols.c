#include <prelude.h>
#include <lib/stdio.h>
#include "symbols.h"

#include "interpreter.h"
#include "intrinsic.h"
#include "types.h"

void SymbolScope_delete(SymbolScope *self)
{
    _Vector_delete(&self->t.nodes);
    _Vector_delete(&self->t.entries);
}

Symbols Symbols_new(Types *types, Slice(SymbolInitializer) init, Slice(SymbolInitializer_intrin) initIntrin, Allocator *allocator)
{
    Symbols ret = (Symbols) {
            .allocator = allocator,
            .scopes = Vector_new(SymbolScope, allocator)
    };
    Symbols *self = &ret;
    Symbols_push(self);
    Slice_loop(&init, i) {
        SymbolInitializer it = *Slice_at(&init, i);
        Symbols_define(self, it.id, (Symbol) {
                .file = Ref_null,
                .type = it.value.type,
                .value = it.value
        });
    }
    Slice_loop(&initIntrin, i) {
        SymbolInitializer_intrin it = *Slice_at(&initIntrin, i);
        Ref(Type) T = it.value->load(types);
        Symbols_define(self, it.id, (Symbol) {
                .file = Ref_null,
                .type = T,
                .value = {
                        .type = T,
                        .u.intrinsic.value = it.value,
                        .flags = {
                                .abstract = it.flags.abstract,
                                .intrinsic = true,
                        }
                },
        });
    }
    return ret;
}

static SymbolScope SymbolScope_new(Ref(SymbolScope) parent, Allocator *allocator)
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
    Trie_set((void *) &self->t, ident.bytes, &val);
}

void Symbols_push(Symbols *symbols)
{
    Allocator *allocator = symbols->allocator;
    size_t size = Vector_size(&symbols->scopes);
    Ref(SymbolScope) parent = !size ? (Ref(SymbolScope)) Ref_null : (Ref(SymbolScope)) Ref_fromIndex(size - 1);
    SymbolScope newscope = SymbolScope_new(parent, allocator);
    Vector_push(&symbols->scopes, newscope);
}

void Symbols_pop(Symbols *symbols)
{
    SymbolScope_delete(Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1));
    Vector_pop(&symbols->scopes);
}

bool Symbols_lookup(const Symbols *symbols, String ident, Symbol *out)
{
    Ref(SymbolScope) scope = Ref_fromIndex(Vector_size(&symbols->scopes) - 1);
    for (;;) {
        SymbolScope *it = Vector_at(&symbols->scopes, Ref_toIndex(scope));
        if (SymbolScope_get(it, ident, out)) {
            return true;
        }
        const Ref(SymbolScope) next = it->parent;
        if (Ref_eq(scope, next)) {
            return false;
        }
        scope = next;
    }
}

void Symbols_define(Symbols *symbols, String ident, Symbol sym)
{
    SymbolScope *top = Vector_at(&symbols->scopes, Vector_size(&symbols->scopes) - 1);
    SymbolScope_set(top, ident, sym);
}
