#include <system.h>

#include <lib/fs.h>
#include <lib/stdio.h>
#include <lib/string.h>

#include "compilation.h"
#include "env.h"
#include "phases/04-compile/compile.h"

#include <compiler/targets/c.h>

#include "intrinsics/debug/puti.h"
#include "intrinsics/debug/puts.h"
#include "intrinsics/types/func.h"
#include "intrinsics/cond.h"
#include "intrinsics/define.h"
#include "intrinsics/do.h"
#include "intrinsics/emit.h"
#include "intrinsics/extern.h"
#include "intrinsics/func.h"
#include "intrinsics/if.h"
#include "intrinsics/include.h"
#include "intrinsics/minus.h"
#include "intrinsics/plus.h"
#include "intrinsics/set.h"
#include "intrinsics/untyped.h"
#include "intrinsics/while.h"
#include "symbols.h"

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

    File *out = fs_stdout();
    FilePath inputFilePath = fs_path_from_native(Vector_data(&args)[1]);
    File *outputFile = Vector_size(&args) >= 3 ? fs_open(fs_path_from_native(Vector_data(&args)[2]), STR("w")) : out;

    compilation_t _compilation = (compilation_t) {
            .debug = out,
            .flags.print_parse = flags.print_parse,
            .flags.print_flatten = flags.print_flatten,
            .flags.print_eval = flags.print_eval,
            .files = Vector_new(),
    };
    compilation_t *compilation = &_compilation;
    compilation_file_ref mainFile = compilation_include(compilation, inputFilePath);

    types_t _types = types_new();
    types_t *types = &_types;
    symbols_t _symbols = symbols_new(types, Slice_of(InitialSymbol, (InitialSymbol[16]) {
            {.id = STR("#puti"), .value = &intrin_debug_puti},
            {.id = STR("#puts"), .value = &intrin_debug_puts},
            {.id = STR("#types/func"), .value = &intrin_types_func},
            {.id = STR("#cond"), .value = &intrin_cond},
            {.id = STR("#define"), .value = &intrin_define},
            {.id = STR("#do"), .value = &intrin_do},
            {.id = STR("#emit"), .value = &intrin_emit},
            {.id = STR("#extern"), .value = &intrin_extern},
            {.id = STR("#func"), .value = &intrin_func},
            {.id = STR("#if"), .value = &intrin_if},
            {.id = STR("#include"), .value = &intrin_include},
            {.id = STR("-"), .value = &intrin_minus},
            {.id = STR("+"), .value = &intrin_plus},
            {.id = STR("#set"), .value = &intrin_set},
            {.id = STR("#untyped"), .value = &intrin_untyped, .flags.abstract = true},
            {.id = STR("#while"), .value = &intrin_while},
    }));
    symbols_t *symbols = &_symbols;
    Buffer prelude = Vector_new();
    File *preludeFile = Buffer_asFile(&prelude);
    Env env = (Env) {
            .compilation = compilation,
            .types = types,
            .symbols = symbols,
            .out = out,
            .preludeBuf = &prelude,
            .prelude = preludeFile,
    };
    compilation_begin(compilation, mainFile, env);

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(out, STR("RUN:\n---\n"));
        }
        sym_t entry;
        bool hasMain = sym_lookup(symbols, STR("main"),
        &entry);
        (void) hasMain;
        assert(hasMain && type_lookup(types, entry.type)->kind == TYPE_FUNCTION && "main is a function");
        func_call(env, entry.value, (Slice(value_t)) {._begin = NULL, ._end = NULL,}, (compilation_node_ref) {.file = {0}, .node = {0}});
    } else {
        if (flags.print_compile) {
            fprintf_s(out, STR("COMPILE:\n-------\n"));
        }
        Buffer outBuf = Vector_new();
        File *f = flags.buffer ? Buffer_asFile(&outBuf) : outputFile;
        do_compile((compile_input) {
                .out = f,
                .target = &target_c,
                .env = env,
        });
        if (flags.buffer) {
            fs_close(f);
            fprintf_raw(outputFile, Buffer_toSlice(&outBuf));
        }
        if (outputFile != out) {
            fs_close(outputFile);
        }
    }

    fprintf_s(out, STR("\n"));
    Slice_loop(&Vector_toSlice(compilation_file_ptr_t, &env.compilation->files), i) {
        compilation_file_ptr_t it = Vector_data(&env.compilation->files)[i];
        Vector_delete(&it->path.parts);
        free(it->content);
        Vector_delete(&it->tokens);
        Vector_delete(&it->nodes);
        free(it);
    }
    Vector_delete(&env.compilation->files);
    Vector_delete(&env.types->all);
    Slice_loop(&Vector_toSlice(sym_scope_t, &env.symbols->scopes), i) {
        Trie(sym_t) it = Vector_data(&env.symbols->scopes)[i].t;
        Vector_delete(&it.nodes);
        Vector_delete(&it.entries);
    }
    Vector_delete(&env.symbols->scopes);
    fs_close(preludeFile);
    Vector_delete(&prelude);
    return EXIT_SUCCESS;
}
