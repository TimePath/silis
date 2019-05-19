#pragma once

#include <lib/fs.h>
#include <lib/stdio.h>

#define ParserError(id, _) \
    _(id, UnexpectedEscape, { size_t c; }) \
/**/

ENUM(ParserError)

void ParserError_print(ParserError self, File *out);
