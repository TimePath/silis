#pragma once

#include "node-fwd.h"

#include <lib/string.h>
#include <lib/vector.h>

#include "compilation-fwd.h"
#include "token-fwd.h"

typedef enum {
    NODE_INVALID,
    /**
     * Generated at runtime
     * Index to address of expression in flat AST
     */ NODE_REF,
    /**
     * (
     */ NODE_LIST_BEGIN,
    /**
     * )
     */ NODE_LIST_END,
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

struct node_s {
    node_e kind;
    uint8_t padding[4];
    const token_t *token;
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
            compilation_node_ref value;
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
};

typedef struct {
    void *compilation;
    Slice(node_t) nodes;
    size_t _i, _n;
} nodelist;

bool nodelist_next(nodelist *self, compilation_node_ref *out);

nodelist nodelist_iterator(Slice(node_t) list, void *env);

Slice(node_t) node_list_children(const node_t *list);

compilation_node_ref node_deref(const compilation_t *compilation, compilation_node_ref node);

void node_print(FILE *f, Slice(node_t) nodes);
