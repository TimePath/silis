#include <prelude.h>

#include <lib/env.h>
#include <lib/fs.h>
#include <lib/fs/memoryfile.h>
#include <lib/misc.h>
#include <lib/stdio.h>
#include <lib/string.h>

#include <interpreter/interpreter.h>
#include <interpreter/symbols.h>

#include <compiler/intrinsics/debug/puti.h>
#include <compiler/intrinsics/debug/puts.h>
#include <compiler/intrinsics/types/func.h>
#include <compiler/intrinsics/cond.h>
#include <compiler/intrinsics/define.h>
#include <compiler/intrinsics/do.h>
#include <compiler/intrinsics/emit.h>
#include <compiler/intrinsics/extern.h>
#include <compiler/intrinsics/func.h>
#include <compiler/intrinsics/if.h>
#include <compiler/intrinsics/include.h>
#include <compiler/intrinsics/minus.h>
#include <compiler/intrinsics/plus.h>
#include <compiler/intrinsics/set.h>
#include <compiler/intrinsics/untyped.h>
#include <compiler/intrinsics/while.h>

#include <compiler/targets/c.h>
#include <compiler/targets/js.h>

#include "emit.h"

static Target *target(String targetName)
{
#define X(id) if (String_equals(targetName, STR(#id))) return &target_##id;
    X(c);
    X(js);
#undef X
    unreachable(return NULL);
}

static void emit(Interpreter *interpreter, compile_file *it, FileSystem *fs_out);

size_t main(Env env);

size_t main(Env env)
{
    Allocator *allocator = env.allocator;
    Slice(String) args = env.args;
    #define arg(name) 1
    #define opt(name) 0
    assert(Slice_size(&args) >= arg(self) + arg(target) + arg(dir_in) + arg(dir_out) + arg(main));
    #undef opt
    #undef arg
    struct {
        bool run : 1;
        bool print_lex : 1;
        bool print_parse : 1;
        bool print_eval : 1;
        bool print_emit : 1;
        bool print_run : 1;
        BIT_PADDING(uint8_t, 2);
    } flags = {
            .run = false,
            .print_lex = true,
            .print_parse = true,
            .print_eval = true,
            .print_emit = true,
            .print_run = true,
    };

    File *stdout = env.stdout;
    String targetName = *Slice_at(&args, 1);
    FileSystem _fs_in, *fs_in = &_fs_in;
    FileSystem_newroot(env.fs, FilePath_from_native(*Slice_at(&args, 2), allocator), fs_in);
    FileSystem _fs_out, *fs_out = &_fs_out;
    FileSystem_newroot(env.fs, FilePath_from_native(*Slice_at(&args, 3), allocator), fs_out);
    FilePath inputFilePath = FilePath_from_native(*Slice_at(&args, 4), allocator);

    Types _types, *types = &_types;
    Types_new(types, allocator);

    Symbols _symbols = Symbols_new(types,
    Slice_of(SymbolInitializer, ((SymbolInitializer[2]) {
            {.id = STR("#types/string"), .value = (Value) {
                    .type = types->t_type,
                    .u.Type = types->t_string,
                    .flags = { .intrinsic = true, }
            }},
            {.id = STR("#types/int"), .value = (Value) {
                    .type = types->t_type,
                    .u.Type = types->t_int,
                    .flags = { .intrinsic = true, }
            }},
    })),
    Slice_of(SymbolInitializer_intrin, ((SymbolInitializer_intrin[16]) {
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
            {.id = STR("#untyped"), .value = &intrin_untyped, .flags = { .abstract = true }},
            {.id = STR("#while"), .value = &intrin_while},
    })),
    allocator
    );
    Symbols *symbols = &_symbols;
    Interpreter _interpreter = (Interpreter) {
            .allocator = allocator,
            .fs_in = fs_in,
            .compilation = {
                    .debug = stdout,
                    .flags = {
                            .print_lex = flags.print_lex,
                            .print_parse = flags.print_parse,
                            .print_eval = flags.print_eval,
                    },
                    .files = Vector_new(InterpreterFilePtr, allocator),
                    ._padding = {0},
            },
            .types = types,
            .symbols = symbols,
            .out = stdout,
    };
    Interpreter *interpreter = &_interpreter;

    Ref(InterpreterFilePtr) mainFile = Interpreter_load(interpreter, fs_in, inputFilePath);
    Interpreter_eval(interpreter, mainFile);

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(stdout, STR("RUN:\n---\n"));
        }
        Symbol entry;
        bool hasMain = Symbols_lookup(symbols, STR("main"), &entry);
        (void) hasMain;
        assert(hasMain && Types_lookup(types, entry.type)->kind.val == Type_Function && "main is a function");
        func_call(interpreter, entry.value, (Slice(Value)) {._begin.r = NULL, ._end = NULL,}, (InterpreterFileNodeRef) {.file = Ref_null, .node = Ref_null});
    } else {
        if (flags.print_emit) {
            fprintf_s(stdout, STR("EMIT:\n----\n"));
        }
        emit_output ret = do_emit((emit_input) {
                .interpreter = interpreter,
                .target = target(targetName),
        });
        for (size_t processed = 0, n = Vector_size(&ret.files), stage = 0; processed < n; ++stage) {
            Vector_loop(compile_file, &ret.files, i) {
                compile_file *it = Vector_at(&ret.files, i);
                if (it->stage != stage) continue;
                emit(interpreter, it, fs_out);
                processed += 1;
            }
        }
        Vector_delete(compile_file, &ret.files);
    }

    fprintf_s(stdout, STR("\n"));
    Vector_delete(InterpreterFilePtr, &interpreter->compilation.files);
    Vector_delete(Type, &interpreter->types->all);
    Vector_delete(SymbolScope, &interpreter->symbols->scopes);
    FileSystem_delete(fs_out);
    FileSystem_delete(fs_in);
    return 0;
}

static void emit(Interpreter *interpreter, compile_file *it, FileSystem *fs_out)
{
    Allocator *allocator = interpreter->allocator;
    File *stdout = interpreter->out;
    Buffer bufRoot = Buffer_new(allocator);
    {
        FilePath_to_native(interpreter->fs_in->root, &bufRoot);
        String slash = STR("/");
        Vector_push(&bufRoot, *Slice_at(&slash.bytes, 0));
    }
    const String strRoot = String_fromSlice(Buffer_toSlice(&bufRoot), ENCODING_DEFAULT);
    Buffer bufBase = Buffer_new(allocator);
    {
        FilePath_to_native(Interpreter_lookup_file(interpreter, it->file)->path, &bufBase);
        String dot = STR(".");
        Vector_push(&bufBase, *Slice_at(&dot.bytes, 0));
        const size_t sizeBytes = String_sizeBytes(it->ext);
        _Vector_push(&bufBase, sizeBytes, String_begin(it->ext), sizeBytes);
    }
    const String strBase = String_fromSlice(Buffer_toSlice(&bufBase), ENCODING_DEFAULT);
    FilePath outputFilePath = FilePath_from_native(strBase, allocator);
    File *outputFile = FileSystem_open(fs_out, outputFilePath, STR("w"));
    {
        fprintf_s(stdout, STR(" * file://"));
        fprintf_s(stdout, strRoot);
        fprintf_s(stdout, strBase);
        fprintf_s(stdout, STR("\n"));
    }
    fprintf_raw(outputFile, Buffer_toSlice(it->content));
    File_close(outputFile);
    FilePath_delete(&outputFilePath);
    Buffer_delete(&bufBase);
    Buffer_delete(&bufRoot);

}
