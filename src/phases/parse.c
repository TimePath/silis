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

char_rule_e parse_chars[256] = {
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

static bool parse_is_space(native_char_t c) {
    return parse_chars[(uint8_t) c] == CHAR_WS;
}

static size_t parse_atom(ctx_t *ctx, buffer_t prog) {
    const native_char_t *begin = (const native_char_t *) prog.data, *end = begin;
    bool number = true;
    for (native_char_t c; (c = *end); ++end) {
        const char_rule_e r = parse_chars[(uint8_t) c];
        if (r <= CHAR_WS) {
            break;
        }
        if (r != CHAR_DIGIT) {
            number = false;
        }
    }
    const string_view_t name = (string_view_t) {.begin = (const uint8_t *) begin, .end = (const uint8_t *) end};
    if (number) {
        ast_push(ctx, (node_t) {
                .kind = NODE_INTEGRAL,
                .u.integral.value = strtoul(begin, NULL, 10),
        });
    } else {
        ast_push(ctx, (node_t) {
                .kind = NODE_ATOM,
                .u.atom.value = name,
        });
    }
    return str_size(name);
}

static size_t parse_string(ctx_t *ctx, buffer_t prog) {
    native_char_t *out = (native_char_t *) prog.data;
    const native_char_t *begin = (const native_char_t *) prog.data, *end = begin;
    for (native_char_t c; (c = *end); ++end) {
        switch (c) {
            default:
                *out++ = c;
                break;
            case '"':
                goto done;
            case '\\':
                switch (c = *++end) {
                    default:
                        assert(false);
                    case '\\':
                    case '"':
                        *out++ = c;
                        break;
#define X(code, replacement) case code: *out++ = replacement; break
                    X('n', '\n');
                    X('r', '\r');
                    X('t', '\t');
#undef X
                }
                break;
        }
    }
    done:;
    const string_view_t value = (string_view_t) {.begin = (const uint8_t *) begin, .end = (const uint8_t *) out};
    ast_push(ctx, (node_t) {
            .kind = NODE_STRING,
            .u.string.value = value,
    });
    return str_size((string_view_t) {.begin = (const uint8_t *) begin, .end = (const uint8_t *) end}) + 1;
}

size_t parse_list(ctx_t *ctx, buffer_t prog) {
    const size_t tok = ast_parse_push(ctx);
    native_char_t *begin = (native_char_t *) prog.data, *end = begin;
    size_t ret;
    for (native_char_t c; (c = *end); ++end) {
        if (parse_is_space(c)) {
            continue;
        }
        native_char_t *next = end + 1;
        const size_t count = (size_t) (next - begin);
        buffer_t rest = (buffer_t) {.size = prog.size - count, .data = (uint8_t *) next};
        switch (c) {
            case ';':
                while (*++end != '\n');
                break;
            case '(':
            case '[': // sugar
                end += parse_list(ctx, rest);
                break;
            case ')':
            case ']': // sugar
                ret = count;
                goto done;
            case '"':
                end += parse_string(ctx, rest);
                break;
            default:
                rest.data--;
                rest.size++;
                end += parse_atom(ctx, rest) - 1;
                break;
        }
    }
    ret = (size_t) (end - begin) - 1;
    done:;
    ast_parse_pop(ctx, tok);
    return ret;
}
