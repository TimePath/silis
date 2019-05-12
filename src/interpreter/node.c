#include <system.h>
#include "node.h"
#include "env.h"

bool nodelist_next(nodelist *self, compilation_node_ref *out)
{
    if (self->_i < self->_n) {
        size_t i = self->_i++;
        if (out) {
            *out = nodelist_get(self, i);
        }
        return true;
    }
    return false;
}

compilation_node_ref nodelist_get(nodelist *self, size_t index)
{
    return (compilation_node_ref) {
            .file = self->head.file,
            .node = { .id = self->head.node.id + index }
    };
}

nodelist nodelist_iterator(const compilation_t *compilation, compilation_node_ref list)
{
    const node_t *node = compilation_node(compilation, list);
    assert(node->kind == NODE_LIST_BEGIN);
    return (nodelist) {
            .compilation = compilation,
            .head = (compilation_node_ref) { .file = list.file, .node = { .id = list.node.id + 1 }},
            ._i = 0,
            ._n = node->u.list.size,
    };
}

compilation_node_ref node_deref(const compilation_t *compilation, compilation_node_ref ref)
{
    const node_t *it = compilation_node(compilation, ref);
    if (it->kind == NODE_REF) {
        ref.node.id = it->u.ref.value;
        assert(compilation_node(compilation, ref)->kind == NODE_LIST_BEGIN && "references refer to lists");
    }
    return ref;
}
