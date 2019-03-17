#pragma once

#include <lib/vector.h>
#include <lib/slice.h>

typedef struct node_s node_t;

Slice_instantiate(node_t);
Vector_instantiate(node_t);

typedef const node_t *node_t_ptr;
Slice_instantiate(node_t_ptr);
