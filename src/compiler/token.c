#include <system.h>
#include "token.h"

#include <lib/stdio.h>

typedef struct {
    File *out;
    Slice(token_t) tokens;
} token_print_ctx_t;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    uint8_t _padding[6];
} token_print_state_t;

static token_print_state_t _token_print(token_print_ctx_t *ctx, token_print_state_t state, const token_t *it, size_t id);

void token_print(File *f, Slice(token_t) tokens)
{
    token_print_ctx_t ctx = {
            .out = f,
            .tokens = tokens,
    };
    token_print_state_t state = (token_print_state_t) {
            .depth = 0,
            .needLine = false,
            .needTab = false,
    };
    Slice_loop(&tokens, i) {
        const token_t *it = &Slice_data(&tokens)[i];
        state = _token_print(&ctx, state, it, i + 1);
    }
}

static void _token_print_indent(token_print_ctx_t *ctx, token_print_state_t *state)
{
    if (state->needLine) {
        fprintf_s(ctx->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(ctx->out, String_indent(2 * state->depth));
        state->needTab = false;
    }
}

static token_print_state_t _token_print(token_print_ctx_t *ctx, token_print_state_t state, const token_t *it, size_t id)
{
    if (it->kind == TOKEN_INVALID) {
        assert(false);
        return state;
    }
    if (it->kind == TOKEN_LIST_BEGIN) {
        _token_print_indent(ctx, &state);
        ++state.depth;
        state.needTab = true;
        state.needLine = true;
        fprintf_s(ctx->out, STR("("));
        fprintf_s(ctx->out, STR(" ;"));
        {
            {
                fprintf_s(ctx->out, STR(" .id = "));
                fprintf_zu(ctx->out, id);
                fprintf_s(ctx->out, STR(","));
            }
        }
    } else if (it->kind == TOKEN_LIST_END) {
        --state.depth;
        _token_print_indent(ctx, &state);
        state.needTab = true;
        state.needLine = true;
        fprintf_s(ctx->out, STR(")"));
        fprintf_s(ctx->out, STR(" ;"));
        {
            {
                fprintf_s(ctx->out, STR(" .id = "));
                fprintf_zu(ctx->out, id);
                fprintf_s(ctx->out, STR(","));
            }
        }
    } else {
        _token_print_indent(ctx, &state);
        switch (it->kind) {
            case TOKEN_ATOM:
                fprintf_s(ctx->out, STR("`"));
                fprintf_s(ctx->out, it->u.atom.value);
                fprintf_s(ctx->out, STR("`"));
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case TOKEN_INTEGRAL:
                fprintf_zu(ctx->out, it->u.integral.value);
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case TOKEN_STRING:
                fprintf_s(ctx->out, STR("\""));
                fprintf_s(ctx->out, it->u.string.value);
                fprintf_s(ctx->out, STR("\""));
                fprintf_s(ctx->out, STR(" ;"));
                {
                    {
                        fprintf_s(ctx->out, STR(" .id = "));
                        fprintf_zu(ctx->out, id);
                        fprintf_s(ctx->out, STR(","));
                    }
                }
                break;
            case TOKEN_INVALID:
            case TOKEN_LIST_BEGIN:
            case TOKEN_LIST_END:
                assert(false);
                break;
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}
