#pragma once

#include "node-fwd.h"

#include <lib/fs.h>
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
    uint8_t _padding[4];
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
            size_t begin;
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

#define node_t_delete(self) ((void) (self))

typedef struct {
    const compilation_t *compilation;
    compilation_node_ref head;
    size_t _i, _n;
} nodelist;

bool nodelist_next(nodelist *self, compilation_node_ref *out);

compilation_node_ref nodelist_get(nodelist *self, size_t index);

nodelist nodelist_iterator(const compilation_t *compilation, compilation_node_ref list);

compilation_node_ref node_deref(const compilation_t *compilation, compilation_node_ref node);

void node_print(File *f, Slice(node_t) nodes);
