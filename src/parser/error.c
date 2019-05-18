#include <system.h>
#include "error.h"

void ParserError_print(ParserError err, File *out) {
    switch (err.kind) {
        case ParserError_INVALID:
            unreachable();
            break;
        case ParserError_UnexpectedEscape:
            fprintf_s(out, STR("Unexpected escape: '\\"));
            fs_write(out, Slice_of(uint8_t, (uint8_t[]) { (uint8_t) err.u.UnexpectedEscape.c }));
            fprintf_s(out, STR("'"));
            break;
    }
    fs_flush(out);
}
