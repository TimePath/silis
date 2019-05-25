#include <system.h>
#include "lexer.h"

#include <lib/result.h>

#include "error.h"

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
} Lexer_char_rule_e;

static Lexer_char_rule_e Lexer_chars[] = {
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

Result_instantiate(size_t, ParserError);

typedef struct {
    Vector(Token) tokens;
} Lexer;

static size_t Lexer_atom(Lexer *self, String prog);

static Lexer_char_rule_e Lexer_char(size_t c);

static Result(size_t, ParserError) Lexer_list(Lexer *self, String prog);

static Result(size_t, ParserError) Lexer_string(Lexer *self, String prog);

static void Lexer_yield(Lexer *self, Token it);

static void Lexer_yield(Lexer *self, Token it)
{
    Vector_push(&self->tokens, it);
}

static Lexer_char_rule_e Lexer_char(size_t c)
{
    return c < ARRAY_LEN(Lexer_chars)
           ? Lexer_chars[c]
           : CHAR_INVALID;
}

static size_t Lexer_atom(Lexer *self, String prog)
{
    const StringEncoding *enc = prog.encoding;
    bool number = true;
    Slice(uint8_t) it = prog.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); it = enc->next(it)) {
        const size_t c = enc->get(it);
        const Lexer_char_rule_e r = Lexer_char(c);
        if (r <= CHAR_WS) {
            break;
        }
        if (r != CHAR_DIGIT) {
            number = false;
        }
    }
    const String atom = String_fromSlice(
            (Slice(uint8_t)) {
                    ._begin.r = String_begin(prog),
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
        Lexer_yield(self, (Token) {
                .kind.val = Token_Integral,
                .u.Integral.value = val,
        });
    } else {
        Lexer_yield(self, (Token) {
                .kind.val = Token_Atom,
                .u.Atom.value = atom,
        });
    }
    return String_sizeUnits(atom);
}

static Result(size_t, ParserError) Lexer_string(Lexer *self, String prog)
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
                                ((ParserError) {.kind.val = ParserError_UnexpectedEscape, .u.UnexpectedEscape = {.c = c}})
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
    const String value = String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = (const uint8_t *) out}, enc);
    Lexer_yield(self, (Token) {
            .kind.val = Token_String,
            .u.String.value = value,
    });
    size_t ret = 1 + enc->count_units((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}) + 1;
    return (Result(size_t, ParserError)) Result_ok(ret);
}

static Result(size_t, ParserError) Lexer_list(Lexer *self, String prog)
{
    const StringEncoding *enc = prog.encoding;
    Lexer_yield(self, (Token) {
            .kind.val = Token_ListBegin,
    });
    const uint8_t *begin = String_begin(prog);
    size_t ret;
    Slice(uint8_t) it = prog.bytes;
    for (Slice(uint8_t) next; (void) (next = enc->next(it)), Slice_begin(&it) != Slice_end(&it); it = next) {
        size_t c = enc->get(it);
        if (Lexer_char(c) == CHAR_WS) {
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
                Result(size_t, ParserError) res = Lexer_list(self, String_fromSlice(next, enc));
                if (!res.is.ok) return res;
                next = enc->skip_units(it, res.ret.val);
                break;
            }
            case ')':
            case ']': // sugar
            case '}': // sugar
                ret = 1 + enc->count_units((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}) + 1;
                goto done;
            case '"':
            {
                Result(size_t, ParserError) res = Lexer_string(self, String_fromSlice(next, enc));
                if (!res.is.ok) return res;
                next = enc->skip_units(it, res.ret.val);
                break;
            }
            default:
                next = enc->skip_units(it, Lexer_atom(self, String_fromSlice(it, enc)));
                break;
        }
    }
    ret = enc->count_units((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}); // EOF
    done:;
    Lexer_yield(self, (Token) {
            .kind.val = Token_ListEnd,
    });
    return (Result(size_t, ParserError)) Result_ok(ret);
}

Result(Vector(Token), ParserError) silis_parser_lex(silis_parser_lex_input in)
{
    Allocator *allocator = in.allocator;
    Lexer lexer = (Lexer) {
            .tokens = Vector_new(allocator),
    };
    Result(size_t, ParserError) res = Lexer_list(&lexer, in.source);
    if (!res.is.ok) return (Result(Vector(Token), ParserError)) Result_err(res.ret.err);
    return (Result(Vector(Token), ParserError)) Result_ok(lexer.tokens);
}
