#include <system.h>
#include "trie.h"

#define Trie_node(self, i) ((TrieNode *) (((uint8_t *) (Vector_data(&(self)->nodes))) + (TrieNode_Size(self) * (i))))

bool Trie_get(Trie(bool) *self, Slice(uint8_t) key, void *out)
{
    TrieNode *n = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = Slice_data(&key)[i];
        const uint16_t idx = n->children[b];
        if (idx == 0) {
            return false;
        }
        n = Trie_node(self, idx);
    }
    if (!n->initialised) {
        return false;
    }
    memcpy(out, TrieNode_Value(n), self->t_size);
    return true;
}

static const size_t uint16_max = (uint16_t) -1;

void Trie_set(Trie(bool) *self, Slice(uint8_t) key, void *value)
{
    TrieNode *n = Trie_node(self, 0);
    Slice_loop(&key, i) {
        const uint8_t b = Slice_data(&key)[i];
        const uint16_t idx = n->children[b];
        if (idx != 0) {
            n = Trie_node(self, idx);
            continue;
        }
        const size_t end = Vector_size(&self->nodes);
        assert(end < uint16_max && "size is constrained");
        n->children[b] = (uint16_t) end;
        TrieNode x = {0};
        (Vector_push)(TrieNode_Size(self), &self->nodes, sizeof(x), &x);
        n = Trie_node(self, end);
    }
    memcpy(TrieNode_Value(n), value, self->t_size);
    if (!n->initialised) {
        n->initialised = true;
        TrieEntry e = (TrieEntry) {
                .key = key,
                .value = (uint16_t) (Vector_size(&self->nodes) - 1),
        };
        Vector_push(&self->entries, e);
    }
}
