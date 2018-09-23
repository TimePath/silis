#include <system.h>
#include "node.h"

Slice(node_t) node_list_children(const node_t *list)
{
    assert(list->kind == NODE_LIST_BEGIN);
    size_t n = list->u.list.size;
    const node_t *begin = list + 1;
    return (Slice(node_t)) {._begin = begin, ._end = begin + n};
}

const node_t *node_get(const Vector(node_t) *nodes, node_id ref)
{
    return &Vector_data(nodes)[ref.val];
}

node_id node_ref(const node_t *it, const Vector(node_t) *nodes)
{
    assert(it >= Vector_data(nodes) &&
           it <= &Vector_data(nodes)[Vector_size(nodes) - 1]);
    return (node_id) {.val = (size_t) (it - Vector_data(nodes))};
}

const node_t *node_deref(const node_t *it, const Vector(node_t) *nodes)
{
    if (it->kind != NODE_REF) {
        return it;
    }
    const node_t *ret = node_get(nodes, it->u.ref.value);
    assert(ret->kind != NODE_REF && "no double refs");
    return ret;
}
