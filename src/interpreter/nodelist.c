#include <prelude.h>
#include "nodelist.h"

#include <lib/misc.h>

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
            .node = Ref_fromIndex(Ref_toIndex(self->head.node) + index),
    };
}

NodeList NodeList_iterator(const Interpreter *compilation, InterpreterFileNodeRef list)
{
    const Node *node = Interpreter_lookup_file_node(compilation, list);
    assert(node->kind.val == Node_ListBegin);
    return (NodeList) {
            .compilation = compilation,
            .head = (InterpreterFileNodeRef) { .file = list.file, .node = Ref_fromIndex(Ref_toIndex(list.node) + 1)},
            ._i = 0,
            ._n = node->u.ListBegin.size,
    };
}
