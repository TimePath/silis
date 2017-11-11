#include "parse.h"

#include <stdlib.h>
#include <assert.h>

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

static bool parse_is_space(char c) {
    return c == '\t' || c == '\n' || c == ' ';
}

static bool parse_is_digit(char c) {
    switch (c) {
        default:
            return false;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
    }
}

static bool parse_is_alpha(char c) {
    switch (c) {
        default:
            return false;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
            return true;
    }
}

static bool parse_is_special(char c) {
    switch (c) {
        default:
            return false;
        case '_':
        case '$':
        case '#':
        case '?':
            return true;
    }
}

static bool parse_is_sym(char c) {
    switch (c) {
        default:
            return false;
        case '@':
        case '+':
        case '-':
        case '!':
        case '~':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
        case '&':
        case '^':
        case '|':
        case '=':
            return true;
    }
}

static size_t parse_atom(ctx_t *ctx, buffer_t prog) {
    const char *begin = prog.data, *end = begin;
    bool number = true;
    for (char c; (c = *end); ++end) {
        if (parse_is_digit(c)) {
            continue;
        }
        if (parse_is_alpha(c)) {
            number = false;
            continue;
        }
        if (parse_is_special(c)) {
            number = false;
            continue;
        }
        if (parse_is_sym(c)) {
            number = false;
            continue;
        }
        break;
    }
    const string_view_t name = (string_view_t) {.begin = begin, .end = end};
    if (number) {
        ast_push(ctx, (node_t) {
                .type = NODE_INTEGRAL,
                .text = name,
                .u.integral.value = strtoul(begin, NULL, 10),
        });
    } else {
        ast_push(ctx, (node_t) {
                .type = NODE_ATOM,
                .text = name,
                .u.atom.value = name,
        });
    }
    return str_size(name);
}

static size_t parse_string(ctx_t *ctx, buffer_t prog) {
    char *out = prog.data;
    const char *begin = prog.data, *end = begin;
    for (char c; (c = *end); ++end) {
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
    const string_view_t value = (string_view_t) {.begin = begin, .end = out};
    ast_push(ctx, (node_t) {
            .type = NODE_STRING,
            .text = value,
            .u.string.value = value,
    });
    return str_size((string_view_t) {.begin = begin, .end = end}) + 1;
}

size_t parse_list(ctx_t *ctx, buffer_t prog) {
    const size_t tok = ast_parse_push(ctx);
    char *begin = prog.data, *end = begin;
    size_t ret;
    for (char c; (c = *end); ++end) {
        if (parse_is_space(c)) {
            continue;
        }
        char *next = end + 1;
        const size_t count = next - begin;
        buffer_t rest = (buffer_t) {.size = prog.size - count, .data = next};
        switch (c) {
            case ';':
                while (*++end != '\n');
                break;
            case '(':
                end += parse_list(ctx, rest);
                break;
            case ')':
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
    ret = end - begin - 1;
    done:;
    ast_parse_pop(ctx, tok);
    return ret;
}
