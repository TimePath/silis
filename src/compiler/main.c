#include <system.h>

#include <lib/stdio.h>
#include <lib/string.h>
#include <compiler/targets/c.h>

#include "env.h"
#include "phases/01-parse/parse.h"
#include "phases/02-flatten/flatten.h"
#include "phases/03-eval/eval.h"
#include "phases/04-compile/compile.h"

#include "intrinsics/debug/puti.h"
#include "intrinsics/debug/puts.h"
#include "intrinsics/types/func.h"
#include "intrinsics/cond.h"
#include "intrinsics/define.h"
#include "intrinsics/do.h"
#include "intrinsics/extern.h"
#include "intrinsics/func.h"
#include "intrinsics/if.h"
#include "intrinsics/minus.h"
#include "intrinsics/plus.h"
#include "intrinsics/set.h"
#include "intrinsics/while.h"

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
            .print_parse = true,
            .print_flatten = true,
            .print_eval = true,
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

    FILE *out = stdout;
    FILE *o = Vector_size(&args) >= 3 ? fopen(String_begin(Vector_data(&args)[2]), "w") : out;

    if (flags.print_parse) {
        fprintf_s(out, STR("PARSE:\n-----\n"));
    }
    parse_output parse = do_parse((parse_input) {
            .source = fileStr,
    });
    if (flags.print_parse) {
        token_print(out, Vector_toSlice(token_t, &parse.tokens));
        fprintf_s(out, STR("\n\n"));
    }

    if (flags.print_flatten) {
        fprintf_s(out, STR("FLATTEN:\n-------\n"));
    }
    flatten_output flatten = do_flatten((flatten_input) {
            .tokens = parse.tokens,
    });
    if (flags.print_flatten) {
        node_print(out, Vector_toSlice(node_t, &flatten.nodes));
        fprintf_s(out, STR("\n"));
    }

    types_t types = types_new();
    symbols_t symbols = symbols_new(&types, Slice_of(InitialSymbol, (InitialSymbol[13]) {
            {.id = STR("#puti"), .value = &intrin_debug_puti},
            {.id = STR("#puts"), .value = &intrin_debug_puts},
            {.id = STR("#types/func"), .value = &intrin_types_func},
            {.id = STR("#cond"), .value = &intrin_cond},
            {.id = STR("#define"), .value = &intrin_define},
            {.id = STR("#do"), .value = &intrin_do},
            {.id = STR("#extern"), .value = &intrin_extern},
            {.id = STR("#func"), .value = &intrin_func},
            {.id = STR("#if"), .value = &intrin_if},
            {.id = STR("-"), .value = &intrin_minus},
            {.id = STR("+"), .value = &intrin_plus},
            {.id = STR("#set"), .value = &intrin_set},
            {.id = STR("#while"), .value = &intrin_while},
    }));

    Env env = (Env) {
            .types = &types,
            .symbols = &symbols,
            .nodes = &flatten.nodes,
            .stdout = out,
    };

    if (flags.print_eval) {
        fprintf_s(out, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .env = env,
            .entry = flatten.entry,
    });
    if (flags.print_eval) {
        fprintf_s(out, STR("\n"));
    }

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(out, STR("RUN:\n---\n"));
        }
        sym_t entry;
        bool hasMain = sym_lookup(&symbols, STR("main"), &entry);
        (void) hasMain;
        assert(hasMain && type_lookup(&types, entry.type)->kind == TYPE_FUNCTION && "main is a function");
        func_call(env, entry.value, (Slice(value_t)) { ._begin = NULL, ._end = NULL, }, NULL);
    } else {
        if (flags.print_compile) {
            fprintf_s(out, STR("COMPILE:\n-------\n"));
        }
        Buffer outBuf = Vector_new();
        FILE *f = flags.buffer ? Buffer_asFile(&outBuf) : o;
        do_compile((compile_input) {
                .out = f,
                .target = &target_c,
                .env = env,
        });
        if (flags.buffer) {
            fclose(f);
            fprintf_raw(o, Buffer_toSlice(&outBuf));
        }
        if (o != out) {
            fclose(o);
        }
    }

    fprintf_s(out, STR("\n"));
    return EXIT_SUCCESS;
}
