#pragma once

#include <lib/fs.h>
#include <lib/slice.h>
#include <lib/string.h>
#include <lib/vector.h>

#include "token.h"

#define Node(id, _) \
    /** Generated at runtime. Index to address of expression in AST buffer */ \
    _(id, Ref, { \
        size_t value; \
    }) \
    /** ( */ \
    _(id, ListBegin, { \
        const Token *token; \
        size_t size; \
        /** indices for first and last child; 0 means null */ \
        size_t begin; \
        size_t end; \
    }) \
    /** ) */ \
    _(id, ListEnd, { \
        const Token *token; \
        size_t begin; \
    }) \
    /** ) */ \
    _(id, Atom, { \
        const Token *token; \
        String value; \
    }) \
    /** ) */ \
    _(id, Integral, { \
        const Token *token; \
        size_t value; \
    }) \
    /** ) */ \
    _(id, String, { \
        const Token *token; \
        String value; \
    }) \
/**/

ENUM(Node)

Slice_instantiate(Node);
Vector_instantiate(Node);

#define Node_delete(self) ((void) (self))

void silis_parser_print_nodes(Slice(Node) nodes, File *f, Allocator *allocator);
