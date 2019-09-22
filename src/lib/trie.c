#include <prelude.h>
#include "trie.h"

#include <system.h>

#include "misc.h"

typedef TrieNode(TriePlaceholder) TrieNode;
#define Trie_node(self, i) Vector_at(&(self)->nodes, i)

typedef TrieEntry(TriePlaceholder) TrieEntry;
#define Trie_entrysize(self) ((self)->_sizeofEntry)
#define Trie_entry(self, i) _Vector_at(TrieEntry, Trie_entrysize(self), &(self)->entries, i)
#define Trie_entryvalue(self) ((TriePlaceholder *) (((uint8_t *) (self)) + OFFSETOF(TrieEntry, value)))

bool Trie_get(Trie *self, Slice(uint8_t) key, void *value)
{
    TrieNode *it = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = *Slice_at(&key, i);
        const TrieRef idx = it->children[b];
        if (idx != 0) {
            it = Trie_node(self, idx);
            continue;
        }
        return false;
    }
    if (!it->entry) {
        return false;
    }
    TrieEntry *e = Trie_entry(self, it->entry - 1);
    libsystem_memcpy(value, Trie_entryvalue(e), self->_sizeofT);
    return true;
}

static const size_t TrieRef_max = (TrieRef) -1;

void Trie_set(Trie *self, Slice(uint8_t) key, void *value)
{
    TrieNode *it = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = *Slice_at(&key, i);
        const TrieRef idx = it->children[b];
        if (idx != 0) {
            it = Trie_node(self, idx);
            continue;
        }
        const size_t end = Vector_size(&self->nodes);
        (void) TrieRef_max;
        assert(end < TrieRef_max && "size is constrained");
        it->children[b] = (TrieRef) end;
        TrieNode n = {.children = {0}, .entry = 0};
        Vector_push(&self->nodes, n);
        it = Trie_node(self, end);
    }
    if (!it->entry) {
        it->entry = (TrieRef) (Vector_size(&self->entries) + 1);
        TrieEntry e = (TrieEntry) {
                .key = key,
        };
        _Vector_push(Trie_entrysize(self), &self->entries, sizeof(e), &e, 1);
    }
    TrieEntry *e = Trie_entry(self, it->entry - 1);
    libsystem_memcpy(Trie_entryvalue(e), value, self->_sizeofT);
}
