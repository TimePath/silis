#include "ctx.h"
#include "phases/parse.h"
#include "phases/flatten.h"
#include "phases/print.h"
#include "phases/eval.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char *argv[]) {
    struct {
        bool print_parse;
        bool print_flatten;
    } args = {
            .print_parse = true,
            .print_flatten = true,
    };
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    const long ret = ftell(file);
    if (ret < 0) return 1;
    const size_t len = (size_t) ret;
    fseek(file, 0, SEEK_SET);
    char buf[len + 1];
    fread(buf, len, 1, file);
    buf[len] = 0;
    fclose(file);

    ctx_t ctx_ = {
            .source.buffer = (buffer_t) {.data = buf, .size = len},
    };
    ctx_t *ctx = &ctx_;
    ctx_init(ctx);

    printf("\nPARSE:\n-----\n");
    parse_list(ctx, ctx->source.buffer);
    if (args.print_parse) {
        print_state_t state = {0};
        for (size_t i = 0; i < ctx->parse.out.size; ++i) {
            const node_t *it = &ctx->parse.out.data[i];
            state = print(state, it);
        }
    }

    printf("\nFLATTEN:\n-------\n");
    do_flatten(ctx);
    if (args.print_flatten) {
        print_state_t state = {0};
        vec_loop(ctx->flatten.out, i, 1) {
            const node_t *it = &ctx->flatten.out.data[i];
            if (it->type == NODE_LIST_BEGIN) {
                printf(";; var_%lu\n", i);
            }
            state = print(state, it);
            if (it->type == NODE_LIST_END) {
                printf("\n");
                state = (print_state_t) {0};
            }
        }
    }

    printf("\nEVAL:\n----\n");
    do_eval(ctx);

    return EXIT_SUCCESS;
}
