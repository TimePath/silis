#include <system.h>
#include "node.h"
#include "interpreter.h"

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

nodelist nodelist_iterator(const Interpreter *compilation, compilation_node_ref list)
{
    const Node *node = compilation_node(compilation, list);
    assert(node->kind == Node_ListBegin);
    return (nodelist) {
            .compilation = compilation,
            .head = (compilation_node_ref) { .file = list.file, .node = { .id = list.node.id + 1 }},
            ._i = 0,
            ._n = node->u.ListBegin.size,
    };
}

compilation_node_ref node_deref(const Interpreter *compilation, compilation_node_ref ref)
{
    const Node *it = compilation_node(compilation, ref);
    if (it->kind == Node_Ref) {
        ref.node.id = it->u.Ref.value;
        assert(compilation_node(compilation, ref)->kind == Node_ListBegin && "references refer to lists");
    }
    return ref;
}
