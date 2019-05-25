#include <system.h>
#include "parser.h"

typedef struct {
    Slice(Token) tokens;
    Vector(Node) nodes;
    Vector(Node) stack;
} Parser;

static Node Parser_convert(const Token *it);

static size_t Parser_parse(Parser *self, const Token *it);

static void Parser_yield(Parser *self, Node it);

static Node Parser_convert(const Token *it)
{
    switch (it->kind.val) {
        case Token_INVALID:
        case Token_ListBegin:
        case Token_ListEnd:
            unreachable();
            break;
        case Token_Atom:
            return (Node) {
                    .kind.val = Node_Atom,
                    .u.Atom = {
                            .token = it,
                            .value = it->u.Atom.value,
                    },
            };
        case Token_Integral:
            return (Node) {
                    .kind.val = Node_Integral,
                    .u.Integral = {
                            .token = it,
                            .value = it->u.Integral.value,
                    },
            };
        case Token_String:
            return (Node) {
                    .kind.val = Node_String,
                    .u.String = {
                            .token = it,
                            .value = it->u.String.value,
                    },
            };
    }
    return (Node) {
            .kind.val = Node_INVALID,
    };
}

// depth-first search
static size_t Parser_parse(Parser *self, const Token *it)
{
    const Token *begin = it;
    if (it->kind.val != Token_ListBegin) {
        Node n = Parser_convert(it);
        Vector_push(&self->stack, n);
        return 1;
    }
    const size_t argv_begin = Vector_size(&self->stack);
    {
        ++it; // skip begin
        while (it->kind.val != Token_ListEnd) {
            it += Parser_parse(self, it);
        }
    }
    const Token *end = it;
    // just parsed a full expression
    const size_t argc = Vector_size(&self->stack) - argv_begin;
    size_t autoid = Vector_size(&self->nodes) + 1;
    {
        const Node header = (Node) {
                .kind.val = Node_ListBegin,
                .u.ListBegin = {
                        .token = begin,
                        .begin = !argc ? 0 : autoid + 1,
                        .end = !argc ? 0 : autoid + argc,
                        .size = argc,
                },
        };
        Parser_yield(self, header);
        for (size_t i = 0; i < argc; ++i) {
            Parser_yield(self, *Vector_at(&self->stack, argv_begin + i));
        }
        for (size_t i = 0; i < argc; ++i) {
            Vector_pop(&self->stack);
        }
        const Node footer = (Node) {
                .kind.val = Node_ListEnd,
                .u.ListEnd = {
                        .token = end,
                        .begin = autoid,
                },
        };
        Parser_yield(self, footer);
    }
    const Node ret = (Node) {
            .kind.val = Node_Ref,
            .u.Ref.value = autoid,
    };
    Vector_push(&self->stack, ret);
    return 1 + (size_t) (end - begin);
}

static void Parser_yield(Parser *self, Node it)
{
    Vector_push(&self->nodes, it);
}

silis_parser_parse_output silis_parser_parse(silis_parser_parse_input in)
{
    Allocator *allocator = in.allocator;
    Parser parser = (Parser) {
            .tokens = in.tokens,
            .nodes = Vector_new(allocator),
            .stack = Vector_new(allocator),
    };
    Parser_parse(&parser, Slice_at(&parser.tokens, 0));
    assert(Vector_size(&parser.stack) == 1 && "stack contains result");
    Node *it = Vector_at(&parser.stack, 0);
    assert(it->kind.val == Node_Ref && "result is a reference");
    size_t root_id = it->u.Ref.value;
    Vector_pop(&parser.stack);
    return (silis_parser_parse_output) {.nodes = parser.nodes, .root_id = root_id};
}
