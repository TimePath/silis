#include <system.h>
#include "error.h"

void ParserError_print(ParserError self, File *out) {
    switch (self.kind) {
        case ParserError_INVALID:
            unreachable();
            break;
        case ParserError_UnexpectedEscape:
            fprintf_s(out, STR("Unexpected escape: '\\"));
            fs_write(out, Slice_of(uint8_t, (uint8_t[]) { (uint8_t) self.u.UnexpectedEscape.c }));
            fprintf_s(out, STR("'"));
            break;
    }
    fs_flush(out);
}
