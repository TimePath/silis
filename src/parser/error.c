#include <prelude.h>
#include "error.h"

void ParserError_print(ParserError self, File *out) {
    switch (self.kind.val) {
        case ParserError_INVALID:
            unreachable(break);
        case ParserError_UnexpectedEscape:
            fprintf_s(out, STR("Unexpected escape: '\\"));
            File_write(out, Slice_of(uint8_t, (Array(uint8_t, 1)) { (uint8_t) self.u.UnexpectedEscape.c }));
            fprintf_s(out, STR("'"));
            break;
    }
    File_flush(out);
}
