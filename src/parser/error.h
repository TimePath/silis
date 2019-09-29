#pragma once

#include <lib/fs.h>
#include <lib/stdio.h>

#define ParserError(_, case) \
    case(_, UnexpectedEscape, struct { size_t c; }) \
/**/

ADT_instantiate(ParserError);
#undef ParserError

void ParserError_print(ParserError self, File *out);
