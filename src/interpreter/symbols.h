#pragma once

#include <lib/slice.h>
#include <lib/trie.h>

#include "intrinsic.h"

#include "symbol.h"
#include "types.h"

Trie_instantiate(Symbol);
typedef struct {
    Trie(Symbol) t;
    size_t parent;
} SymbolScope;

void SymbolScope_delete(SymbolScope *self);

Slice_instantiate(SymbolScope);
Vector_instantiate(SymbolScope);
typedef struct Symbols {
    Allocator *allocator;
    Vector(SymbolScope) scopes;
} Symbols;

typedef struct {
    String id;
    value_t value;
} SymbolInitializer;
Slice_instantiate(SymbolInitializer);

typedef struct {
    String id;
    Intrinsic *value;
    struct {
        bool abstract : 1;
        uint8_t _padding : 7;
    } flags;
    PADDING(7)
} SymbolInitializer_intrin;
Slice_instantiate(SymbolInitializer_intrin);

Symbols Symbols_new(Allocator *allocator, Types *types, Slice(SymbolInitializer) init, Slice(SymbolInitializer_intrin) initIntrin);

void Symbols_push(Symbols *symbols);

void Symbols_pop(Symbols *symbols);

bool Symbols_lookup(const Symbols *symbols, String ident, Symbol *out);

void Symbols_define(Symbols *symbols, String ident, Symbol sym);
