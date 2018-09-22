#include <system.h>
#include "node.h"

const node_t *node_get(const Vector(node_t) *nodes, node_id ref)
{
    return &Vector_data(nodes)[ref.val];
}

node_id node_ref(const node_t *it, const Vector(node_t) *nodes)
{
    assert(it >= Vector_data(nodes) &&
           it <= &Vector_data(nodes)[Vector_size(nodes) - 1]);
    return (node_id) {(size_t) (it - Vector_data(nodes))};
}

const node_t *node_deref(const node_t *it, const Vector(node_t) *nodes)
{
    if (it->kind == NODE_REF) {
        const node_t *ret = node_get(nodes, it->u.ref.value);
        assert(ret->kind != NODE_REF && "no double refs");
        return ret;
    }
    return it;
}
