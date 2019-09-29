#include <prelude.h>
#include "trie.h"

#include <system.h>

#include "misc.h"

typedef TrieNode(TriePlaceholder) TrieNode;
#define Trie_node(self, i) Vector_at(&(self)->nodes, i)

typedef TrieEntry(TriePlaceholder) TrieEntry;
#define Trie_entry(self, i) _Vector_at(TrieEntry, &(self)->entries, i)
#define Trie_entryvalue(self) (&(self)->value)

bool Trie_get(Trie *self, Slice(uint8_t) key, void *value)
{
    TrieNode *it = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = *Slice_at(&key, i);
        const Ref(TrieNode) ref = it->children[b];
        if (Ref_toBool(ref)) {
            it = Trie_node(self, Ref_toIndex(ref));
            continue;
        }
        return false;
    }
    if (!Ref_toBool(it->entry)) {
        return false;
    }
    TrieEntry *e = Trie_entry(self, Ref_toIndex(it->entry));
    libsystem_memcpy(value, Trie_entryvalue(e), self->_sizeofT);
    return true;
}

void Trie_set(Trie *self, Slice(uint8_t) key, void *value)
{
    TrieNode *it = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = *Slice_at(&key, i);
        const Ref(TrieNode) ref = it->children[b];
        if (Ref_toBool(ref)) {
            it = Trie_node(self, Ref_toIndex(ref));
            continue;
        }
        const size_t end = Vector_size(&self->nodes);
        assert(Ref_fromIndexCheck(TrieNode, end) && "size is constrained");
        it->children[b] = (Ref(TrieNode)) Ref_fromIndex(end);
        TrieNode n = {.children = {Ref_null}, .entry = Ref_null};
        Vector_push(&self->nodes, n);
        it = Trie_node(self, end);
    }
    if (!Ref_toBool(it->entry)) {
        it->entry = (Ref(TrieNode)) Vector_ref(&self->entries, Vector_size(&self->entries));
        TrieEntry e = (TrieEntry) {
                .key = key,
        };
        Vector_push(&self->entries, e);
    }
    TrieEntry *e = Trie_entry(self, Ref_toIndex(it->entry));
    libsystem_memcpy(Trie_entryvalue(e), value, self->_sizeofT);
}
