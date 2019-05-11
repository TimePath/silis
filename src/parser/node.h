#pragma once

#include <lib/vector.h>
#include <lib/slice.h>

#include "token.h"

typedef struct node_s node_t;

Slice_instantiate(node_t);
Vector_instantiate(node_t);

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
    PADDING(4)
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
            size_t value;
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

void node_print(Allocator *allocator, File *f, Slice(node_t) nodes);
