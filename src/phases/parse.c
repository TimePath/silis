#include "../system.h"
#include "parse.h"

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

static bool parse_is_space(ascii_codepoint c) {
    return parse_char(c) == CHAR_WS;
}

static size_t parse_atom(ctx_t *ctx, string_view_t prog) {
    const ascii_unit *begin = str_begin(prog), *end = str_end(prog), *it = begin;
    bool number = true;
    for (; it != end; it = ascii_next(it)) {
        const ascii_codepoint c = ascii_get(it);
        const char_rule_e r = parse_char(c);
        if (r <= CHAR_WS) {
            break;
        }
        if (r != CHAR_DIGIT) {
            number = false;
        }
    }
    const string_view_t atom = str_from(begin, it);
    if (number) {
        ast_push(ctx, (node_t) {
                .kind = NODE_INTEGRAL,
                .u.integral.value = strtoul(ascii_native(str_begin(atom)), NULL, 10),
        });
    } else {
        ast_push(ctx, (node_t) {
                .kind = NODE_ATOM,
                .u.atom.value = atom,
        });
    }
    return ascii_unit_count(str_begin(atom), str_end(atom));
}

static size_t parse_string(ctx_t *ctx, string_view_t prog) {
    const ascii_unit *begin = str_begin(prog), *end = str_end(prog), *it = begin;
    ascii_unit *out = (ascii_unit *) begin; // XXX: mutation, but only decreases size
    for (; it != end; it = ascii_next(it)) {
        ascii_codepoint c = ascii_get(it);
        switch (c) {
            default:
                out = ascii_set(out, c);
                break;
            case '"':
                goto done;
            case '\\':
                switch (c = ascii_get(it = ascii_next(it))) {
                    default:
                        assert(false);
                    case '\\':
                    case '"':
                        out = ascii_set(out, c);
                        break;
#define X(code, replacement) case code: out = ascii_set(out, replacement); break
                    X('n', '\n');
                    X('r', '\r');
                    X('t', '\t');
#undef X
                }
                break;
        }
    }
    done:;
    const string_view_t value = str_from(begin, out);
    ast_push(ctx, (node_t) {
            .kind = NODE_STRING,
            .u.string.value = value,
    });
    return 1 + ascii_unit_count(begin, it) + 1;
}

size_t parse_list(ctx_t *ctx, string_view_t prog) {
    const size_t tok = ast_parse_push(ctx);
    const ascii_unit *begin = str_begin(prog), *end = str_end(prog), *it = begin;
    size_t ret;
    for (const ascii_unit *next; next = ascii_next(it), it != end; it = next) {
        ascii_codepoint c = ascii_get(it);
        if (parse_is_space(c)) {
            continue;
        }
        switch (c) {
            case ';':
                for (;;) {
                    c = ascii_get(next);
                    next = ascii_next(next);
                    if (c == '\n') {
                        break;
                    }
                }
                break;
            case '(':
            case '[': // sugar
                next = ascii_unit_skip(it, parse_list(ctx, str_from(next, end)));
                break;
            case ')':
            case ']': // sugar
                ret = 1 + ascii_unit_count(begin, it) + 1;
                goto done;
            case '"':
                next = ascii_unit_skip(it, parse_string(ctx, str_from(next, end)));
                break;
            default:
                next = ascii_unit_skip(it, parse_atom(ctx, str_from(it, end)));
                break;
        }
    }
    ret = ascii_unit_count(begin, it); // EOF
    done:;
    ast_parse_pop(ctx, tok);
    return ret;
}
