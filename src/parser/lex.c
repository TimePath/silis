#include <system.h>
#include "lex.h"

#include <lib/result.h>

#include "error.h"

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

 SYMBOL: '@' | '_' | '$' | '#' | '?' | '.'
    | '+' | '-'
    | '!' | '~'
    | '*' | '/' | '%'
    | '<' | '>'
    | '&' | '^' | '|'
    | '='
    ;

*/

Result_instantiate(size_t, ParserError);

typedef struct {
    Vector(token_t) tokens;
} lex_context;

static void lex_yield(lex_context *ctx, token_t it)
{
    Vector_push(&ctx->tokens, it);
}

static Result(size_t, ParserError) lex_list(lex_context *ctx, String prog);

Result(Vector(token_t), ParserError) silis_parser_lex(lex_input in)
{
    Allocator *allocator = in.allocator;
    lex_context ctx = (lex_context) {
        .tokens = Vector_new(allocator),
    };
    Result(size_t, ParserError) res = lex_list(&ctx, in.source);
    if (!res.ok) return (Result(Vector(token_t), ParserError)) Result_err(res.err);
    return (Result(Vector(token_t), ParserError)) Result_ok(ctx.tokens);
}

typedef enum {
    CHAR_INVALID,

    CHAR_WS,
#define CHAR_WS(_) \
    _('\t') \
    _('\r') \
    _('\n') \
    _(' ') \
    /**/

    CHAR_SPECIAL,
#define CHAR_SPECIAL(_) \
    _('_') \
    _('$') \
    _('#') \
    _('?') \
    _('.') \
    /**/

    CHAR_SYM,
#define CHAR_SYM(_) \
    _('@') \
    _('+') \
    _('-') \
    _('!') \
    _('~') \
    _('*') \
    _('/') \
    _('%') \
    _('<') \
    _('>') \
    _('&') \
    _('^') \
    _('|') \
    _('=') \
    /**/

    CHAR_DIGIT,
#define CHAR_DIGIT(_) \
    _('0') \
    _('1') \
    _('2') \
    _('3') \
    _('4') \
    _('5') \
    _('6') \
    _('7') \
    _('8') \
    _('9') \
    /**/

    CHAR_ALPHA,
#define CHAR_ALPHA(_) \
    _('a') _('A') \
    _('b') _('B') \
    _('c') _('C') \
    _('d') _('D') \
    _('e') _('E') \
    _('f') _('F') \
    _('g') _('G') \
    _('h') _('H') \
    _('i') _('I') \
    _('j') _('J') \
    _('k') _('K') \
    _('l') _('L') \
    _('m') _('M') \
    _('n') _('N') \
    _('o') _('O') \
    _('p') _('P') \
    _('q') _('Q') \
    _('r') _('R') \
    _('s') _('S') \
    _('t') _('T') \
    _('u') _('U') \
    _('v') _('V') \
    _('w') _('W') \
    _('x') _('X') \
    _('y') _('Y') \
    _('z') _('Z') \
    /**/
} lex_char_rule_e;

static lex_char_rule_e lex_chars[] = {
#define CASE(_) [_] = CHAR_WS,
        CHAR_WS(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_SPECIAL,
        CHAR_SPECIAL(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_SYM,
        CHAR_SYM(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_DIGIT,
        CHAR_DIGIT(CASE)
#undef CASE
#define CASE(_) [_] = CHAR_ALPHA,
        CHAR_ALPHA(CASE)
#undef CASE
};

static lex_char_rule_e lex_char(size_t c)
{
    return c < ARRAY_LEN(lex_chars)
           ? lex_chars[c]
           : CHAR_INVALID;
}

static size_t lex_atom(lex_context *ctx, String prog)
{
    const StringEncoding *enc = prog.encoding;
    bool number = true;
    Slice(uint8_t) it = prog.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); it = enc->next(it)) {
        const size_t c = enc->get(it);
        const lex_char_rule_e r = lex_char(c);
        if (r <= CHAR_WS) {
            break;
        }
        if (r != CHAR_DIGIT) {
            number = false;
        }
    }
    const String atom = String_fromSlice(
            (Slice(uint8_t)) {
                    ._begin = String_begin(prog),
                    ._end = Slice_begin(&it),
            },
            enc
    );
    if (number) {
        size_t val = 0;
        for (Slice(uint8_t) d = atom.bytes; Slice_begin(&d) != Slice_end(&d); d = enc->next(d)) {
            const size_t c = enc->get(d);
            val = 10 * val + (c - '0');
        }
        lex_yield(ctx, (token_t) {
                .kind = TOKEN_INTEGRAL,
                .u.integral.value = val,
        });
    } else {
        lex_yield(ctx, (token_t) {
                .kind = TOKEN_ATOM,
                .u.atom.value = atom,
        });
    }
    return String_sizeUnits(atom);
}

static Result(size_t, ParserError) lex_string(lex_context *ctx, String prog)
{
    const StringEncoding *enc = prog.encoding;
    const uint8_t *begin = String_begin(prog);
    // mutation, but only decreases size
    /* XXX */ uint8_t *out = CAST(uint8_t *, const uint8_t *, begin);
    Slice(uint8_t) it = prog.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); it = enc->next(it)) {
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
                        return (Result(size_t, ParserError)) Result_err(
                                ((ParserError) {.kind = ParserError_UnexpectedEscape, .u.UnexpectedEscape = {.c = c}})
                        );
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
    const String value = String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = (const uint8_t *) out}, enc);
    lex_yield(ctx, (token_t) {
            .kind = TOKEN_STRING,
            .u.string.value = value,
    });
    size_t ret = 1 + enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}) + 1;
    return (Result(size_t, ParserError)) Result_ok(ret);
}

static Result(size_t, ParserError) lex_list(lex_context *ctx, String prog)
{
    const StringEncoding *enc = prog.encoding;
    lex_yield(ctx, (token_t) {
            .kind = TOKEN_LIST_BEGIN,
    });
    const uint8_t *begin = String_begin(prog);
    size_t ret;
    Slice(uint8_t) it = prog.bytes;
    for (Slice(uint8_t) next; (void) (next = enc->next(it)), Slice_begin(&it) != Slice_end(&it); it = next) {
        size_t c = enc->get(it);
        if (lex_char(c) == CHAR_WS) {
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
            case '{': // sugar
            {
                Result(size_t, ParserError) res = lex_list(ctx, String_fromSlice(next, enc));
                if (!res.ok) return res;
                next = enc->skip_units(it, res.val);
                break;
            }
            case ')':
            case ']': // sugar
            case '}': // sugar
                ret = 1 + enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}) + 1;
                goto done;
            case '"':
            {
                Result(size_t, ParserError) res = lex_string(ctx, String_fromSlice(next, enc));
                if (!res.ok) return res;
                next = enc->skip_units(it, res.val);
                break;
            }
            default:
                next = enc->skip_units(it, lex_atom(ctx, String_fromSlice(it, enc)));
                break;
        }
    }
    ret = enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}); // EOF
    done:;
    lex_yield(ctx, (token_t) {
            .kind = TOKEN_LIST_END,
    });
    return (Result(size_t, ParserError)) Result_ok(ret);
}
