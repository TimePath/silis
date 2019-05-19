#pragma once

#include <lib/fs.h>
#include <lib/slice.h>
#include <lib/string.h>
#include <lib/vector.h>

#define Token(id, _) \
    /** ( */ \
    _(id, ListBegin, { PADDING(1) }) \
    /** ) */ \
    _(id, ListEnd, { PADDING(1) }) \
    /** A word */ \
    _(id, Atom, { String value; }) \
    /** An integer */ \
    _(id, Integral, { size_t value; }) \
    /** A string */ \
    _(id, String, { String value; }) \
/**/

ENUM(Token)

Slice_instantiate(Token);
Vector_instantiate(Token);

#define Token_delete(self) ((void) (self))

void silis_parser_print_tokens(Slice(Token) tokens, File *f, Allocator *allocator);
