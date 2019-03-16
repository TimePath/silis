#pragma once

#include "macro.h"
#include "vector.h"
#include "slice.h"

#define TrieNode(T) CAT2(TrieNode__, T)
#define TrieNode_instantiate(T) typedef TrieNode_(T) TrieNode(T)
#define TrieNode_(T) \
struct { \
    uint16_t children[256]; \
    bool initialised; \
    uint8_t _padding[7]; \
    T value; \
}

typedef struct {
    uint16_t children[256];
    bool initialised;
    uint8_t _padding[7];
    // T value;
} TrieNode;

#define TrieNode_Value_Offset (sizeof (TrieNode))
#define TrieNode_Value(self) ((void *) (((uint8_t *) (self)) + TrieNode_Value_Offset))
#define TrieNode_Size(self) (TrieNode_Value_Offset + ((self)->t_size))

typedef struct {
    Slice(uint8_t) key;
    uint16_t value;
    uint8_t padding[6];
} TrieEntry;

Vector_instantiate(TrieEntry);
Slice_instantiate(TrieEntry);

#define Trie(T) CAT2(Trie__, T)
#define Trie_instantiate(T) \
    TrieNode_instantiate(T); \
    Vector_instantiate(TrieNode(T)); \
    typedef Trie_(T) Trie(T) \
    /**/
#define Trie_(T) \
struct { \
    size_t t_size; \
    Vector(TrieNode(T)) nodes; \
    Vector(TrieEntry) entries; \
}

#define Trie_new(T, self) \
MACRO_BEGIN \
const TrieNode(T) root = (TrieNode(T)) {.children = {0}, .initialised = false}; \
(self)->t_size = sizeof (T); \
Vector_push(&(self)->nodes, root); \
MACRO_END

typedef struct { uint8_t padding[2]; } TriePlaceholder;
Trie_instantiate(TriePlaceholder);
typedef Trie(TriePlaceholder) AnyTrie;

bool Trie_get(AnyTrie *self, Slice(uint8_t) key, void *out);

void Trie_set(AnyTrie *self, Slice(uint8_t) key, void *value, size_t sizeof_Node);
