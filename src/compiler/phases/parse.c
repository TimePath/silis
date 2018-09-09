#include <system.h>
#include "parse.h"

#include <lib/slice.h>
#include <lib/string.h>

/*

 program: expression* EOF;

 expression: list | string | number | atom;

 list: '(' expression* ')';

 string: '"' atom '"';

 number: DIGIT+;

 atom: (DIGIT | LETTER | SYMBOL)+;

 WS: '\t' | '\n' | ' ';

 COMMENT: ';' .* '\n';

 DIGIT: '0'..'9';

 LETTER: 'a'..'z' | 'A'..'Z';

 SYMBOL: '@' | '_' | '$' | '#' | '?'
    | '+' | '-'
    | '!' | '~'
    | '*' | '/' | '%'
    | '<' | '>'
    | '&' | '^' | '|'
    | '='
    ;

*/

static char_rule_e parse_chars[] = {
#define CASE(_) [_] = CHAR_WS,
        PARSE_WS(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_SPECIAL,
        PARSE_SPECIAL(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_SYM,
        PARSE_SYM(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_DIGIT,
        PARSE_DIGIT(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_ALPHA,
        PARSE_ALPHA(CASE)
#undef CASE
};

char_rule_e parse_char(size_t c) {
    return c < ARRAY_LEN(parse_chars)
           ? parse_chars[c]
           : CHAR_INVALID;
}

static bool parse_is_space(size_t c) {
    return parse_char(c) == CHAR_WS;
}

static size_t parse_atom(ctx_t *ctx, String prog) {
    const StringEncoding *enc = prog.encoding;
    bool number = true;
    Slice(uint8_t) it = prog.bytes;
    for (; it.begin != it.end; it = enc->next(it)) {
        const size_t c = enc->get(it);
        const char_rule_e r = parse_char(c);
        if (r <= CHAR_WS) {
            break;
        }
        if (r != CHAR_DIGIT) {
            number = false;
        }
    }
    const String atom = String_fromSlice((Slice(uint8_t)) {String_begin(prog), it.begin}, enc);
    if (number) {
        ast_push(ctx, (node_t) {
                .kind = NODE_INTEGRAL,
                .u.integral.value = strtoul(String_begin(atom), NULL, 10),
        });
    } else {
        ast_push(ctx, (node_t) {
                .kind = NODE_ATOM,
                .u.atom.value = atom,
        });
    }
    return String_sizeUnits(atom);
}

static size_t parse_string(ctx_t *ctx, String prog) {
    const StringEncoding *enc = prog.encoding;
    const uint8_t *begin = String_begin(prog);
    /* XXX */ uint8_t *out = (uint8_t *) begin; // mutation, but only decreases size
    Slice(uint8_t) it = prog.bytes;
    for (; it.begin != it.end; it = enc->next(it)) {
        size_t c = enc->get(it);
        switch (c) {
            default:
                /* XXX */ *out++ = (uint8_t) c;
                break;
            case '"':
                goto done;
            case '\\':
                switch (c = enc->get(it = enc->next(it))) {
                    default:
                        assert(false);
                    case '\\':
                    case '"':
                        /* XXX */ *out++ = (uint8_t) c;
                        break;
#define X(code, replacement) case code: /* XXX */ *out++ = (uint8_t) (replacement); break
                    X('n', '\n');
                    X('r', '\r');
                    X('t', '\t');
#undef X
                }
                break;
        }
    }
    done:;
    const String value = String_fromSlice((Slice(uint8_t)) {begin, (const uint8_t *) out}, enc);
    ast_push(ctx, (node_t) {
            .kind = NODE_STRING,
            .u.string.value = value,
    });
    return 1 + enc->count_units((Slice(uint8_t)) {begin, it.begin}) + 1;
}

size_t parse_list(ctx_t *ctx, String prog) {
    const StringEncoding *enc = prog.encoding;
    const size_t tok = ast_parse_push(ctx);
    const uint8_t *begin = String_begin(prog);
    size_t ret;
    Slice(uint8_t) it = prog.bytes;
    for (Slice(uint8_t) next; next = enc->next(it), it.begin != it.end; it = next) {
        size_t c = enc->get(it);
        if (parse_is_space(c)) {
            continue;
        }
        switch (c) {
            case ';':
                for (;;) {
                    c = enc->get(next);
                    next = enc->next(next);
                    if (c == '\n') {
                        break;
                    }
                }
                break;
            case '(':
            case '[': // sugar
                next = enc->skip_units(it, parse_list(ctx, String_fromSlice(next, enc)));
                break;
            case ')':
            case ']': // sugar
                ret = 1 + enc->count_units((Slice(uint8_t)) {begin, it.begin}) + 1;
                goto done;
            case '"':
                next = enc->skip_units(it, parse_string(ctx, String_fromSlice(next, enc)));
                break;
            default:
                next = enc->skip_units(it, parse_atom(ctx, String_fromSlice(it, enc)));
                break;
        }
    }
    ret = enc->count_units((Slice(uint8_t)) {begin, it.begin}); // EOF
    done:;
    ast_parse_pop(ctx, tok);
    return ret;
}
