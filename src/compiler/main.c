#include <system.h>

#include <lib/stdio.h>
#include <lib/string.h>

#include "env.h"
#include "phases/01-parse/parse.h"
#include "phases/02-flatten/flatten.h"
#include "phases/03-eval/eval.h"
#include "phases/04-compile/compile.h"
#include "phases/print.h"

#include "intrinsics/debug/puti.h"
#include "intrinsics/debug/puts.h"
#include "intrinsics/types/func.h"
#include "intrinsics/cond.h"
#include "intrinsics/define.h"
#include "intrinsics/do.h"
#include "intrinsics/extern.h"
#include "intrinsics/func.h"
#include "intrinsics/minus.h"
#include "intrinsics/plus.h"

Vector_instantiate(String);

MAIN(main)

#ifdef NDEBUG
#define OUTPUT_BUFFER 1
#else
#define OUTPUT_BUFFER 0
#endif

size_t main(Vector(String)
                    args)
{
    struct {
        bool run : 1;
        bool print_parse : 1;
        bool print_flatten : 1;
        bool print_eval : 1;
        bool print_compile : 1;
        bool print_run : 1;
        bool buffer : 1;
        uint8_t padding : 1;
    } flags = {
            .run = false,
            .print_parse = false,
            .print_flatten = true,
            .print_eval = false,
            .print_compile = true,
            .print_run = true,
            .buffer = OUTPUT_BUFFER,
    };
    FILE *file = fopen(String_begin(Vector_data(&args)[1]), "r");
    fseek(file, 0, SEEK_END);
    const native_long_t ret = ftell(file);
    if (ret < 0) { return 1; }
    const size_t len = (size_t) ret;
    fseek(file, 0, SEEK_SET);
    uint8_t *buf = realloc(NULL, len + 1);
    size_t read = fread(buf, len, 1, file);
    (void) read;
    buf[len] = 0;
    fclose(file);
    String fileStr = String_fromSlice((Slice(uint8_t)) {._begin = buf, ._end = buf + len}, ENCODING_DEFAULT);

    if (flags.print_parse) {
        fprintf_s(stdout, STR("PARSE:\n-----\n"));
    }
    parse_output parse = do_parse((parse_input) {
            .source = fileStr,
    });
    if (flags.print_parse) {
        print_state_t state = print_state_new();
        Slice_loop(&Vector_toSlice(node_t, &parse.tokens), i) {
            const node_t *it = &Vector_data(&parse.tokens)[i];
            state = print(stdout, state, it);
        }
        fprintf_s(stdout, STR("\n\n"));
    }

    if (flags.print_flatten) {
        fprintf_s(stdout, STR("FLATTEN:\n-------\n"));
    }
    flatten_output flatten = do_flatten((flatten_input) {
            .tokens = parse.tokens,
    });
    if (flags.print_flatten) {
        print_state_t state = print_state_new();
        Slice_loop(&Vector_toSlice(node_t, &flatten.nodes), i) {
            if (i < 1) { continue; }
            const node_t *it = &Vector_data(&flatten.nodes)[i];
            if (it->kind == NODE_LIST_BEGIN) {
                fprintf_s(stdout, STR(";; var_"));
                fprintf_zu(stdout, i);
                fprintf_s(stdout, STR("\n"));
            }
            state = print(stdout, state, it);
            if (it->kind == NODE_LIST_END) {
                fprintf_s(stdout, STR("\n"));
                state = print_state_new();
            }
        }
        fprintf_s(stdout, STR("\n"));
    }

    types_t types = types_new();
    symbols_t symbols = symbols_new(&types, Slice_of(InitialSymbol, (InitialSymbol[10]) {
            {.id = STR("#puti"),       .value = intrin_debug_puti},
            {.id = STR("#puts"),       .value = intrin_debug_puts},
            {.id = STR("#types/func"), .value = intrin_types_func},
            {.id = STR("#cond"),       .value = intrin_cond},
            {.id = STR("#define"),     .value = intrin_define},
            {.id = STR("#do"),         .value = intrin_do},
            {.id = STR("#extern"),     .value = intrin_extern},
            {.id = STR("#func"),       .value = intrin_func},
            {.id = STR("-"),           .value = intrin_minus},
            {.id = STR("+"),           .value = intrin_plus},
    }));

    Env env = (Env) {
            .types = &types,
            .symbols = &symbols,
            .nodes = &flatten.nodes,
    };

    if (flags.print_eval) {
        fprintf_s(stdout, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .env = env,
    });
    if (flags.print_eval) {
        fprintf_s(stdout, STR("\n"));
    }

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(stdout, STR("RUN:\n---\n"));
        }
        sym_t entry;
        sym_lookup(&symbols, STR("main"), &entry);
        assert(type_lookup(&types, entry.type)->kind == TYPE_FUNCTION && "main is a function");
        func_call(env, entry.value, NULL);
    } else {
        if (flags.print_compile) {
            fprintf_s(stdout, STR("COMPILE:\n-------\n"));
        }
        FILE *out = stdout;
        Buffer outBuf;
        FILE *f = flags.buffer ? Buffer_asFile(&outBuf) : out;
        do_compile(env, f);
        if (flags.buffer) {
            fclose(f);
            fprintf_raw(out, Buffer_toSlice(&outBuf));
        }
        if (out != stdout) {
            fclose(out);
        }
    }

    fprintf_s(stdout, STR("\n"));
    return EXIT_SUCCESS;
}
