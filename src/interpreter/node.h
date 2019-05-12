#pragma once

#include <lib/fs.h>
#include <lib/string.h>
#include <lib/vector.h>

#include "compilation.h"

typedef struct {
    const compilation_t *compilation;
    compilation_node_ref head;
    size_t _i, _n;
} nodelist;

bool nodelist_next(nodelist *self, compilation_node_ref *out);

compilation_node_ref nodelist_get(nodelist *self, size_t index);

nodelist nodelist_iterator(const compilation_t *compilation, compilation_node_ref list);

compilation_node_ref node_deref(const compilation_t *compilation, compilation_node_ref node);
