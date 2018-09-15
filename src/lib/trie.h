#pragma once

#include "macro.h"
#include "vector.h"
#include "slice.h"

#define TrieNode(T) CAT2(TrieNode__, T)
#define TrieNode_$(T) typedef TrieNode_(T) TrieNode(T)
#define TrieNode_(T) \
struct __attribute__((__packed__)) { \
    uint16_t children[256]; \
    bool initialised; \
    T value; \
}

typedef struct __attribute__((__packed__)) {
    uint16_t children[256];
    bool initialised;
    // T value;
} TrieNode;

#define TrieNode_Value_Offset (sizeof (TrieNode))
#define TrieNode_Value(self) ((void *) (((uint8_t *) (self)) + TrieNode_Value_Offset))
#define TrieNode_Size(self) (TrieNode_Value_Offset + ((self)->t_size))

typedef struct {
    Slice(uint8_t) key;
    uint16_t value;
} TrieEntry;

Vector_$(TrieEntry);
Slice_$(TrieEntry);

#define Trie(T) CAT2(Trie__, T)
#define Trie_$(T) \
    TrieNode_$(T); \
    Vector_$(TrieNode(T)); \
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
const TrieNode(T) root = (TrieNode(T)) {0}; \
(self)->t_size = sizeof (T); \
Vector_push(&(self)->nodes, root); \
MACRO_END

Trie_$(bool);

bool Trie_get(Trie(bool) *self, Slice(uint8_t) key, void *out);

void Trie_set(Trie(bool) *self, Slice(uint8_t) key, void *value);
