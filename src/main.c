#include "system.h"

#include "ctx.h"
#include "phases/parse.h"
#include "phases/flatten.h"
#include "phases/print.h"
#include "phases/eval.h"
#include "phases/compile/compile.h"
#include "intrinsics/func.h"

int main(int argc, const char *argv[]) {
    (void) argc;
    struct {
        bool run : 1;
        bool print_parse : 1;
        bool print_flatten : 1;
        bool print_eval : 1;
        bool print_compile : 1;
        bool print_run : 1;
    } args = {
            .run = false,
            .print_parse = false,
            .print_flatten = true,
            .print_eval = false,
            .print_compile = true,
            .print_run = true,
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

    ctx_t ctx_ = (ctx_t) {0};
    ctx_t *ctx = &ctx_;
    ctx_init(ctx);

    if (args.print_parse) {
        fprintf(stdout, "PARSE:\n-----\n");
    }
    parse_list(ctx, (buffer_t) {.data = buf, .size = len});
    if (args.print_parse) {
        print_state_t state = {0};
        const vec_t(node_t) *out = &ctx->parse.out;
        for (size_t i = 0; i < out->size; ++i) {
            const node_t *it = &out->data[i];
            state = print(stdout, state, it);
        }
        fprintf(stdout, "\n\n");
    }

    if (args.print_flatten) {
        fprintf(stdout, "FLATTEN:\n-------\n");
    }
    do_flatten(ctx);
    if (args.print_flatten) {
        print_state_t state = {0};
        vec_loop(ctx->flatten.out, i, 1) {
            const node_t *it = node_get(ctx, (node_id) {i});
            if (it->kind == NODE_LIST_BEGIN) {
                fprintf(stdout, ";; var_%lu\n", i);
            }
            state = print(stdout, state, it);
            if (it->kind == NODE_LIST_END) {
                fprintf(stdout, "\n");
                state = (print_state_t) {0};
            }
        }
        fprintf(stdout, "\n");
    }

    if (args.print_eval) {
        fprintf(stdout, "EVAL:\n----\n");
    }
    do_eval(ctx);
    if (args.print_eval) {
        fprintf(stdout, "\n");
    }

    if (args.run) {
        if (args.print_run) {
            fprintf(stdout, "RUN:\n---\n");
        }
        const sym_t *entry = sym_lookup(ctx, STR("main"));
        assert(type_lookup(ctx, entry->type)->kind == TYPE_FUNCTION && "main is a function");
        func_call(ctx, entry->value, NULL);
    } else {
        if (args.print_compile) {
            fprintf(stdout, "COMPILE:\n-------\n");
        }
        do_compile(ctx);
    }

    fprintf(stdout, "\n");
    return EXIT_SUCCESS;
}
