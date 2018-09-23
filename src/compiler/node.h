#pragma once

#include <lib/string.h>
#include <lib/vector.h>

typedef enum {
    NODE_INVALID,
    /**
     * (
     */ NODE_LIST_BEGIN,
    /**
     * )
     */ NODE_LIST_END,
    /**
     * Generated at runtime
     * Index to address of expression in flat AST
     */ NODE_REF,
    /**
     * A word
     */ NODE_ATOM,
    /**
     * An integer
     */ NODE_INTEGRAL,
    /**
     * A string
     */ NODE_STRING,
} node_e;

typedef struct {
    size_t val;
} node_id;

typedef struct {
    node_e kind;
    uint8_t padding[4];
    union {
        /// NODE_LIST_BEGIN
        struct {
            size_t size;
            /// indices for first and last child
            /// 0 means null
            size_t begin, end;
        } list;
        /// NODE_LIST_END
        struct {
            uint8_t padding;
        } list_end;
        /// NODE_REF
        struct {
            node_id value;
        } ref;
        /// NODE_ATOM
        struct {
            String value;
        } atom;
        /// NODE_INTEGRAL
        struct {
            size_t value;
        } integral;
        /// NODE_STRING
        struct {
            String value;
        } string;
    } u;
} node_t;

Vector_instantiate(node_t);
Slice_instantiate(node_t);

typedef const node_t *node_t_ptr;
Slice_instantiate(node_t_ptr);

Slice(node_t) node_list_children(const node_t *list);

const node_t *node_get(const Vector(node_t) *nodes, node_id ref);

node_id node_ref(const node_t *it, const Vector(node_t) *nodes);

const node_t *node_deref(const node_t *it, const Vector(node_t) *nodes);
