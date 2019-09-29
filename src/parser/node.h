#pragma once

#include <lib/fs.h>
#include <lib/slice.h>
#include <lib/string.h>
#include <lib/vector.h>

#include "token.h"

Ref_instantiate(Node, size_t);

#define Node(_, case) \
    /** Generated at runtime. Index to address of expression in AST buffer */ \
    case(_, Ref, struct { \
        Ref(Node) value; \
    }) \
    /** ( */ \
    case(_, ListBegin, struct { \
        const Token *token; \
        size_t size; \
        /** indices for first and last child */ \
        Ref(Node) begin; \
        Ref(Node) end; \
    }) \
    /** ) */ \
    case(_, ListEnd, struct { \
        const Token *token; \
        Ref(Node) begin; \
    }) \
    /** ) */ \
    case(_, Atom, struct { \
        const Token *token; \
        String value; \
    }) \
    /** ) */ \
    case(_, Integral, struct { \
        const Token *token; \
        size_t value; \
    }) \
    /** ) */ \
    case(_, String, struct { \
        const Token *token; \
        String value; \
    }) \
/**/

ADT_instantiate(Node);
#undef Node

Slice_instantiate(Node);
Vector_instantiate(Node);

#define Node_delete(self) ((void) (self))

void silis_parser_print_nodes(Slice(Node) nodes, File *f, Allocator *allocator);
