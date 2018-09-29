#include <system.h>
#include "parse.h"

typedef struct {
    size_t list_parent_idx;
    Vector(token_t) tokens;
} parse_ctx_t;

#define parse_ctx_new() (parse_ctx_t) { \
    .list_parent_idx = 0, \
    .tokens = Vector_new(), \
} \
/**/

typedef struct {
    size_t idx;
} ctx_list_memo;

/// begin a new list
/// \param ctx compiler context
/// \return memo for pop
static ctx_list_memo ctx_list_push(parse_ctx_t *ctx);

static void ctx_list_add(parse_ctx_t *ctx, token_t it);

static void ctx_list_pop(parse_ctx_t *ctx, ctx_list_memo memo);

static size_t parse_list(parse_ctx_t *ctx, String prog);

parse_output do_parse(parse_input in)
{
    parse_ctx_t ctx = parse_ctx_new();
    parse_list(&ctx, in.source);
    return (parse_output) {.tokens = ctx.tokens};
}

static ctx_list_memo ctx_list_push(parse_ctx_t *ctx)
{
    const size_t parentIdx = ctx->list_parent_idx;
    Vector(token_t) *tokens = &ctx->tokens;
    ctx->list_parent_idx = Vector_size(tokens);
    token_t it = (token_t) {
            .kind = TOKEN_LIST_BEGIN,
    };
    Vector_push(tokens, it);
    return (ctx_list_memo) {.idx = parentIdx};
}

static void ctx_list_add(parse_ctx_t *ctx, token_t it)
{
    Vector(token_t) *tokens = &ctx->tokens;
    Vector_push(tokens, it);
}

static void ctx_list_pop(parse_ctx_t *ctx, ctx_list_memo memo)
{
    Vector(token_t) *tokens = &ctx->tokens;
    ctx->list_parent_idx = memo.idx;
    token_t it = (token_t) {
            .kind = TOKEN_LIST_END,
    };
    Vector_push(tokens, it);
}

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

#define PARSE_WS(_) \
    _('\t') \
    _('\n') \
    _(' ') \
    /**/

#define PARSE_SPECIAL(_) \
    _('_') \
    _('$') \
    _('#') \
    _('?') \
    _('.') \
    /**/

#define PARSE_SYM(_) \
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

#define PARSE_DIGIT(_) \
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

#define PARSE_ALPHA(_) \
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

typedef enum {
    CHAR_INVALID,
    CHAR_WS,
    CHAR_SPECIAL,
    CHAR_SYM,
    CHAR_DIGIT,
    CHAR_ALPHA,
} parse_char_rule_e;

static parse_char_rule_e parse_chars[] = {
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

static parse_char_rule_e parse_char(size_t c)
{
    return c < ARRAY_LEN(parse_chars)
           ? parse_chars[c]
           : CHAR_INVALID;
}

static size_t parse_atom(parse_ctx_t *ctx, String prog)
{
    const StringEncoding *enc = prog.encoding;
    bool number = true;
    Slice(uint8_t) it = prog.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); it = enc->next(it)) {
        const size_t c = enc->get(it);
        const parse_char_rule_e r = parse_char(c);
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
        ctx_list_add(ctx, (token_t) {
                .kind = TOKEN_INTEGRAL,
                .u.integral.value = strtoul(String_begin(atom), NULL, 10),
        });
    } else {
        ctx_list_add(ctx, (token_t) {
                .kind = TOKEN_ATOM,
                .u.atom.value = atom,
        });
    }
    return String_sizeUnits(atom);
}

static size_t parse_string(parse_ctx_t *ctx, String prog)
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
    const String value = String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = (const uint8_t *) out}, enc);
    ctx_list_add(ctx, (token_t) {
            .kind = TOKEN_STRING,
            .u.string.value = value,
    });
    return 1 + enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}) + 1;
}

static size_t parse_list(parse_ctx_t *ctx, String prog)
{
    const StringEncoding *enc = prog.encoding;
    const ctx_list_memo memo = ctx_list_push(ctx);
    const uint8_t *begin = String_begin(prog);
    size_t ret;
    Slice(uint8_t) it = prog.bytes;
    for (Slice(uint8_t) next; (void) (next = enc->next(it)), Slice_begin(&it) != Slice_end(&it); it = next) {
        size_t c = enc->get(it);
        if (parse_char(c) == CHAR_WS) {
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
                next = enc->skip_units(it, parse_list(ctx, String_fromSlice(next, enc)));
                break;
            case ')':
            case ']': // sugar
            case '}': // sugar
                ret = 1 + enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}) + 1;
                goto done;
            case '"':
                next = enc->skip_units(it, parse_string(ctx, String_fromSlice(next, enc)));
                break;
            default:
                next = enc->skip_units(it, parse_atom(ctx, String_fromSlice(it, enc)));
                break;
        }
    }
    ret = enc->count_units((Slice(uint8_t)) {._begin = begin, ._end = Slice_begin(&it)}); // EOF
    done:;
    ctx_list_pop(ctx, memo);
    return ret;
}
