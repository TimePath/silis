#pragma once

#include <lib/slice.h>
#include <lib/trie.h>

#include "intrinsic.h"
#include "symbol.h"
#include "types.h"

Trie_instantiate(sym_t);
typedef struct {
    Trie(sym_t) t;
    size_t parent;
} sym_scope_t;

Vector_instantiate(sym_scope_t);
typedef struct symbols_s {
    Vector(sym_scope_t) scopes;
} symbols_t;


typedef struct {
    String id;
    Intrinsic value;
} InitialSymbol;
Slice_instantiate(InitialSymbol);
symbols_t symbols_new(types_t *types, Slice(InitialSymbol) init);

void sym_push(symbols_t *symbols, size_t parent);

void sym_pop(symbols_t *symbols);

bool sym_lookup(const symbols_t *symbols, String ident, sym_t *out);

void sym_def(symbols_t *symbols, String ident, sym_t sym);
