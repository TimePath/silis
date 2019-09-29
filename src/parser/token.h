#pragma once

#include <lib/fs.h>
#include <lib/slice.h>
#include <lib/string.h>
#include <lib/vector.h>

Ref_instantiate(Token, size_t);

#define Token(_, case) \
    /** ( */ \
    case(_, ListBegin, struct { PADDING(1); }) \
    /** ) */ \
    case(_, ListEnd, struct { PADDING(1); }) \
    /** A word */ \
    case(_, Atom, String) \
    /** An integer */ \
    case(_, Integral, size_t) \
    /** A string */ \
    case(_, String, String) \
/**/

ADT_instantiate(Token);
#undef Token

Slice_instantiate(Token);
Vector_instantiate(Token);

#define Token_delete(self) ((void) (self))

void silis_parser_print_tokens(Slice(Token) tokens, File *f, Allocator *allocator);
