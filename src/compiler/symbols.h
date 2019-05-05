#pragma once

#include <lib/slice.h>
#include <lib/trie.h>

#include <compiler/intrinsics/_.h>

#include "symbol.h"
#include "types.h"

Trie_instantiate(sym_t);
typedef struct {
    Trie(sym_t) t;
    size_t parent;
} sym_scope_t;

void sym_scope_t_delete(sym_scope_t *self);

Slice_instantiate(sym_scope_t);
Vector_instantiate(sym_scope_t);
typedef struct symbols_s {
    Allocator *_allocator;
    Vector(sym_scope_t) scopes;
} symbols_t;

#define symbols_t_new(allocator) (symbols_t) { \
    ._allocator = allocator, \
    .scopes = Vector_new(allocator), \
} \
/**/

typedef struct {
    String id;
    value_t value;
} InitialSymbol;
Slice_instantiate(InitialSymbol);

typedef struct {
    String id;
    Intrinsic *value;
    struct {
        bool abstract : 1;
        uint8_t _padding : 7;
    } flags;
    PADDING(7)
} InitialSymbol_intrin;
Slice_instantiate(InitialSymbol_intrin);

symbols_t symbols_new(Allocator *allocator, types_t *types, Slice(InitialSymbol) init, Slice(InitialSymbol_intrin) initIntrin);

void sym_push(symbols_t *symbols);

void sym_pop(symbols_t *symbols);

bool sym_lookup(const symbols_t *symbols, String ident, sym_t *out);

void sym_def(symbols_t *symbols, String ident, sym_t sym);
