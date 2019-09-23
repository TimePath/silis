#pragma once

#include "macro.h"
#include "slice.h"
#include "vector.h"

Ref_instantiate(TrieNode, uint16_t);

typedef Ref(TrieNode) TrieRef;

#define TrieNode(T) CAT2(TrieNode__, T)
#define TrieNode_instantiate(T) typedef TrieNode_(T) TrieNode(T)
#define TrieNode_(T) \
struct { \
    TrieRef children[256]; \
    TrieRef entry; \
}

#define TrieEntry(T) CAT2(TrieEntry__, T)
#define TrieEntry_instantiate(T) typedef TrieEntry_(T) TrieEntry(T)
#define TrieEntry_(T) \
struct { \
    Slice(uint8_t) key; \
    T value; \
}

#define Trie(T) CAT2(Trie__, T)
#define Trie_instantiate(T) \
    TrieNode_instantiate(T); \
    Slice_instantiate(TrieNode(T)); \
    Vector_instantiate(TrieNode(T)); \
    TrieEntry_instantiate(T); \
    Slice_instantiate(TrieEntry(T)); \
    Vector_instantiate(TrieEntry(T)); \
    typedef Trie_(T) Trie(T) \
    /**/
#define Trie_(T) \
struct { \
    size_t _sizeofT; \
    size_t _sizeofEntry; \
    Vector(TrieNode(T)) nodes; \
    Vector(TrieEntry(T)) entries; \
}

#define Trie_new(T, allocator, self) \
MACRO_BEGIN \
*(self) = (Trie(T)) { ._sizeofT = sizeof (T), ._sizeofEntry = sizeof (TrieEntry(T)), .nodes = Vector_new(allocator), .entries = Vector_new(allocator), }; \
const TrieNode(T) root = (TrieNode(T)) {.children = {Ref_null}, .entry = Ref_null}; \
Vector_push(&(self)->nodes, root); \
MACRO_END

typedef struct { PADDING(8) } TriePlaceholder;
Trie_instantiate(TriePlaceholder);
typedef Trie(TriePlaceholder) Trie;

bool Trie_get(Trie *self, Slice(uint8_t) key, void *value);

void Trie_set(Trie *self, Slice(uint8_t) key, void *value);
