#include <system.h>
#include "node.h"

#include <lib/stdio.h>

typedef struct {
    Allocator *allocator;
    File *out;
    Slice(Node) nodes;
} NodePrinter;

typedef struct {
    size_t depth;
    bool needLine;
    bool needTab;
    PADDING(6)
} NodePrinterState;

static NodePrinterState NodePrinter_print(NodePrinter *self, NodePrinterState state, const Node *it, size_t id);

static void NodePrinter_print_indent(NodePrinter *self, NodePrinterState *state);

static NodePrinterState NodePrinter_print(NodePrinter *self, NodePrinterState state, const Node *it, size_t id)
{
    if (it->kind.val == Node_INVALID) {
        unreachable();
        return state;
    }
    if (it->kind.val == Node_ListBegin) {
        NodePrinter_print_indent(self, &state);
        ++state.depth;
        fprintf_s(self->out, STR("("));
        fprintf_s(self->out, STR(" ;"));
        {
            {
                fprintf_s(self->out, STR(" .id = "));
                fprintf_zu(self->out, id);
                fprintf_s(self->out, STR(","));
            }
            {
                fprintf_s(self->out, STR(" .children = {"));
                {
                    fprintf_s(self->out, STR(" .begin = "));
                    fprintf_zu(self->out, it->u.ListBegin.begin);
                    fprintf_s(self->out, STR(","));
                }
                {
                    fprintf_s(self->out, STR(" .end = "));
                    fprintf_zu(self->out, it->u.ListBegin.end);
                    fprintf_s(self->out, STR(","));
                }
                {
                    fprintf_s(self->out, STR(" .size = "));
                    fprintf_zu(self->out, it->u.ListBegin.size);
                    fprintf_s(self->out, STR(","));
                }
                fprintf_s(self->out, STR(" },"));
            }
        }
        state.needTab = true;
        state.needLine = true;
    } else if (it->kind.val == Node_ListEnd) {
        --state.depth;
        NodePrinter_print_indent(self, &state);
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
            {
                fprintf_s(self->out, STR(" .begin = "));
                fprintf_zu(self->out, it->u.ListEnd.begin);
                fprintf_s(self->out, STR(","));
            }
        }
    } else {
        NodePrinter_print_indent(self, &state);
        switch (it->kind.val) {
            case Node_INVALID:
            case Node_ListBegin:
            case Node_ListEnd:
                unreachable();
                break;
            case Node_Ref:
                fprintf_s(self->out, STR("var_"));
                fprintf_zu(self->out, it->u.Ref.value);
                fprintf_s(self->out, STR(" ;"));
                {
                    {
                        fprintf_s(self->out, STR(" .id = "));
                        fprintf_zu(self->out, id);
                        fprintf_s(self->out, STR(","));
                    }
                }
                break;
            case Node_Atom:
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
            case Node_Integral:
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
            case Node_String:
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
        }
        state.needTab = true;
        state.needLine = true;
    }
    return state;
}

static void NodePrinter_print_indent(NodePrinter *self, NodePrinterState *state)
{
    Allocator *allocator = self->allocator;
    if (state->needLine) {
        fprintf_s(self->out, STR("\n"));
        state->needLine = false;
    }
    if (state->needTab) {
        fprintf_s(self->out, String_indent(2 * state->depth, allocator));
        state->needTab = false;
    }
}

void silis_parser_print_nodes(Slice(Node) nodes, File *f, Allocator *allocator)
{
    NodePrinter printer = {
            .allocator = allocator,
            .out = f,
            .nodes = nodes,
    };
    NodePrinterState state = (NodePrinterState) {
            .depth = 0,
            .needLine = false,
            .needTab = false,
    };
    Slice_loop(&nodes, i) {
        const Node *it = Slice_at(&nodes, i);
        state = NodePrinter_print(&printer, state, it, i + 1);
        if (it->kind.val == Node_ListEnd) {
            fprintf_s(f, STR("\n"));
            state = (NodePrinterState) {
                    .depth = 0,
                    .needLine = false,
                    .needTab = false,
            };
        }
    }
}
