#include <system.h>

#include <lib/stdio.h>
#include <lib/string.h>

#include "ctx.h"
#include "phases/parse.h"
#include "phases/flatten.h"
#include "phases/print.h"
#include "phases/eval.h"
#include "phases/compile/compile.h"
#include "intrinsics/func.h"

Vector_$(String);

MAIN(main)

size_t main(Vector(String) args) {
    struct {
        bool run : 1;
        bool print_parse : 1;
        bool print_flatten : 1;
        bool print_eval : 1;
        bool print_compile : 1;
        bool print_run : 1;
        uint8_t padding : 2;
    } flags = {
            .run = false,
            .print_parse = false,
            .print_flatten = true,
            .print_eval = false,
            .print_compile = true,
            .print_run = true,
    };
    FILE *file = fopen(String_begin(args.data[1]), "r");
    fseek(file, 0, SEEK_END);
    const native_long_t ret = ftell(file);
    if (ret < 0) return 1;
    const size_t len = (size_t) ret;
    fseek(file, 0, SEEK_SET);
    native_char_t buf[len + 1];
    fread(buf, len, 1, file);
    buf[len] = 0;
    fclose(file);
    String fileStr = String_fromSlice((Slice(void)) {buf, buf + len});

    ctx_t ctx_ = (ctx_t) {0};
    ctx_t *ctx = &ctx_;
    ctx_init(ctx);

    if (flags.print_parse) {
        fprintf_s(stdout, STR("PARSE:\n-----\n"));
    }
    parse_list(ctx, fileStr);
    if (flags.print_parse) {
        print_state_t state = {0};
        const Vector(node_t) *out = &ctx->parse.out;
        for (size_t i = 0; i < out->size; ++i) {
            const node_t *it = &out->data[i];
            state = print(stdout, state, it);
        }
        fprintf_s(stdout, STR("\n\n"));
    }

    if (flags.print_flatten) {
        fprintf_s(stdout, STR("FLATTEN:\n-------\n"));
    }
    do_flatten(ctx);
    if (flags.print_flatten) {
        print_state_t state = {0};
        Slice_loop(Vector_toSlice(node_t, ctx->flatten.out), i) {
            if (i < 1) continue;
            const node_t *it = node_get(ctx, (node_id) {i});
            if (it->kind == NODE_LIST_BEGIN) {
                fprintf_s(stdout, STR(";; var_"));
                fprintf_zu(stdout, i);
                fprintf_s(stdout, STR("\n"));
            }
            state = print(stdout, state, it);
            if (it->kind == NODE_LIST_END) {
                fprintf_s(stdout, STR("\n"));
                state = (print_state_t) {0};
            }
        }
        fprintf_s(stdout, STR("\n"));
    }

    if (flags.print_eval) {
        fprintf_s(stdout, STR("EVAL:\n----\n"));
    }
    do_eval(ctx);
    if (flags.print_eval) {
        fprintf_s(stdout, STR("\n"));
    }

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(stdout, STR("RUN:\n---\n"));
        }
        const sym_t *entry = sym_lookup(ctx, STR("main"));
        assert(type_lookup(ctx, entry->type)->kind == TYPE_FUNCTION && "main is a function");
        func_call(ctx, entry->value, NULL);
    } else {
        if (flags.print_compile) {
            fprintf_s(stdout, STR("COMPILE:\n-------\n"));
        }
        do_compile(ctx);
    }

    fprintf_s(stdout, STR("\n"));
    return EXIT_SUCCESS;
}