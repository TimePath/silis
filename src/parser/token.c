#include <system.h>
#include "token.h"

#include <lib/stdio.h>

typedef struct {
    Allocator *allocator;
    File *out;
    Slice(Token) tokens;
} TokenPrinter;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    PADDING(6)
} TokenPrinterState;

static TokenPrinterState TokenPrinter_print(TokenPrinter *self, TokenPrinterState state, const Token *it, size_t id);

static void TokenPrinter_print_indent(TokenPrinter *self, TokenPrinterState *state);

static TokenPrinterState TokenPrinter_print(TokenPrinter *self, TokenPrinterState state, const Token *it, size_t id)
{
    if (it->kind.val == Token_INVALID) {
        unreachable();
        return state;
    }
    if (it->kind.val == Token_ListBegin) {
        TokenPrinter_print_indent(self, &state);
        ++state.depth;
        state.needTab = true;
        state.needLine = true;
        fprintf_s(self->out, STR("("));
        fprintf_s(self->out, STR(" ;"));
        {
            {
                fprintf_s(self->out, STR(" .id = "));
                fprintf_zu(self->out, id);
                fprintf_s(self->out, STR(","));
            }
        }
    } else if (it->kind.val == Token_ListEnd) {
        --state.depth;
        TokenPrinter_print_indent(self, &state);
        state.needTab = true;
        state.needLine = true;
        fprintf_s(self->out, STR(")"));
        fprintf_s(self->out, STR(" ;"));
        {
            {
                fprintf_s(self->out, STR(" .id = "));
                fprintf_zu(self->out, id);
                fprintf_s(self->out, STR(","));
            }
        }
    } else {
        TokenPrinter_print_indent(self, &state);
        switch (it->kind.val) {
            case Token_Atom:
                fprintf_s(self->out, STR("`"));
                fprintf_s(self->out, it->u.Atom.value);
                fprintf_s(self->out, STR("`"));
                fprintf_s(self->out, STR(" ;"));
                {
                    {
                        fprintf_s(self->out, STR(" .id = "));
                        fprintf_zu(self->out, id);
                        fprintf_s(self->out, STR(","));
                    }
                }
                break;
            case Token_Integral:
                fprintf_zu(self->out, it->u.Integral.value);
                fprintf_s(self->out, STR(" ;"));
                {
                    {
                        fprintf_s(self->out, STR(" .id = "));
                        fprintf_zu(self->out, id);
                        fprintf_s(self->out, STR(","));
                    }
                }
                break;
            case Token_String:
                fprintf_s(self->out, STR("\""));
                fprintf_s(self->out, it->u.String.value);
                fprintf_s(self->out, STR("\""));
                fprintf_s(self->out, STR(" ;"));
                {
                    {
                        fprintf_s(self->out, STR(" .id = "));
                        fprintf_zu(self->out, id);
                        fprintf_s(self->out, STR(","));
                    }
                }
                break;
            case Token_INVALID:
            case Token_ListBegin:
            case Token_ListEnd:
                unreachable();
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}

static void TokenPrinter_print_indent(TokenPrinter *self, TokenPrinterState *state)
{
    Allocator *allocator = self->allocator;
    if (state->needLine) {
        fprintf_s(self->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(self->out, String_indent(allocator, 2 * state->depth));
        state->needTab = false;
    }
}

void silis_parser_print_tokens(Slice(Token) tokens, File *f, Allocator *allocator)
{
    TokenPrinter printer = {
            .allocator = allocator,
            .out = f,
            .tokens = tokens,
    };
    TokenPrinterState state = (TokenPrinterState) {
            .depth = 0,
            .needLine = false,
            .needTab = false,
    };
    Slice_loop(&tokens, i) {
        const Token *it = Slice_at(&tokens, i);
        state = TokenPrinter_print(&printer, state, it, i + 1);
    }
}
