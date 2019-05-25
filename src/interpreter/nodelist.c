#include <system.h>
#include "nodelist.h"

bool NodeList_next(NodeList *self, InterpreterFileNodeRef *out)
{
    if (self->_i < self->_n) {
        size_t i = self->_i++;
        if (out) {
            *out = NodeList_get(self, i);
        }
        return true;
    }
    return false;
}

InterpreterFileNodeRef NodeList_get(NodeList *self, size_t index)
{
    return (InterpreterFileNodeRef) {
            .file = self->head.file,
            .node = { .id = self->head.node.id + index }
    };
}

NodeList NodeList_iterator(const Interpreter *compilation, InterpreterFileNodeRef list)
{
    const Node *node = Interpreter_lookup_file_node(compilation, list);
    assert(node->kind.val == Node_ListBegin);
    return (NodeList) {
            .compilation = compilation,
            .head = (InterpreterFileNodeRef) { .file = list.file, .node = { .id = list.node.id + 1 }},
            ._i = 0,
            ._n = node->u.ListBegin.size,
    };
}
