#include <system.h>

#include <lib/fs.h>
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

MAIN(main)

#ifdef NDEBUG
#define OUTPUT_BUFFER 1
#else
#define OUTPUT_BUFFER 0
#endif

static Target *target(String targetName)
{
#define X(id) if (String_equals(targetName, STR(#id))) return &target_##id;
    X(c);
    X(js);
#undef X
    unreachable();
    return NULL;
}

static void emit(Interpreter *interpreter, File *f, compile_file *it);

typedef struct {
    Allocator interface;
    Allocator *implementation;
    size_t size;
    size_t max_size;
} DebugAllocator;

typedef struct {
    size_t size;
} DebugAllocation;

static void *DebugAllocator_alloc(void *_self, size_t size)
{
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = Allocator_alloc(self->implementation, sizeof(DebugAllocation) + size);
    mem->size = size;
    self->size += size;
    self->max_size = self->size > self->max_size ? self->size : self->max_size;
    return mem + 1;
}

static void *DebugAllocator_realloc(void *_self, void *ptr, size_t size)
{
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = ptr ? ((DebugAllocation *) ptr) - 1 : NULL;
    if (mem) {
        self->size -= mem->size;
    }
    mem = Allocator_realloc(self->implementation, mem, sizeof(DebugAllocation) + size);
    mem->size = size;
    self->size += size;
    self->max_size = self->size > self->max_size ? self->size : self->max_size;
    return mem + 1;
}

static void DebugAllocator_free(void *_self, void *ptr)
{
    if (!ptr) return;
    DebugAllocator *self = (DebugAllocator *) _self;
    DebugAllocation *mem = ((DebugAllocation *) ptr) - 1;
    self->size -= mem->size;
    Allocator_free(self->implementation, mem);
}

size_t main(Allocator *allocator, Slice(String) args)
{
    DebugAllocator debugAllocator = (DebugAllocator) {
            .interface = {.alloc = DebugAllocator_alloc, .realloc = DebugAllocator_realloc, .free = DebugAllocator_free},
            .implementation = allocator,
    };
    allocator = &debugAllocator.interface;
    #define arg(name) 1
    #define opt(name) 0
    assert(Slice_size(&args) >= arg(self) + arg(target) + arg(dir_in) + arg(dir_out) + arg(main) + opt(out));
    #undef opt
    #undef arg
    struct {
        bool run : 1;
        bool print_lex : 1;
        bool print_parse : 1;
        bool print_eval : 1;
        bool print_emit : 1;
        bool print_run : 1;
        bool buffer : 1;
        uint8_t _padding : 1;
    } flags = {
            .run = false,
            .print_lex = true,
            .print_parse = true,
            .print_eval = true,
            .print_emit = true,
            .print_run = true,
            .buffer = OUTPUT_BUFFER,
    };

    File *out = fs_stdout(allocator);
    String targetName = *Slice_at(&args, 1);
    FileSystem _fs_in = fs_root(fs_path_from_native(allocator, *Slice_at(&args, 2)));
    FileSystem *fs_in = &_fs_in;
    FileSystem _fs_out = fs_root(fs_path_from_native(allocator, *Slice_at(&args, 3)));
    FileSystem *fs_out = &_fs_out;
    FilePath inputFilePath = fs_path_from_native(allocator, *Slice_at(&args, 4));
    FilePath outputFilePath;
    File *outputFile = out;
    if (Slice_size(&args) > 5) {
        outputFilePath = fs_path_from_native(allocator, *Slice_at(&args, 5));
        outputFile = fs_open(allocator, fs_out, outputFilePath, STR("w"));
    }

    Types _types = Types_new(allocator);
    Types *types = &_types;

    Symbols _symbols = Symbols_new(allocator, types,
    Slice_of(SymbolInitializer, ((SymbolInitializer[2]) {
            {.id = STR("#types/string"), .value = (Value) {
                    .type = types->t_type,
                    .u.type.value = types->t_string,
                    .flags.intrinsic = true,
            }},
            {.id = STR("#types/int"), .value = (Value) {
                    .type = types->t_type,
                    .u.type.value = types->t_int,
                    .flags.intrinsic = true,
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
            {.id = STR("#untyped"), .value = &intrin_untyped, .flags.abstract = true},
            {.id = STR("#while"), .value = &intrin_while},
    })));
    Symbols *symbols = &_symbols;
    Interpreter _interpreter = (Interpreter) {
            .allocator = allocator,
            .fs_in = fs_in,
            .compilation = {
                    .debug = out,
                    .flags.print_lex = flags.print_lex,
                    .flags.print_parse = flags.print_parse,
                    .flags.print_eval = flags.print_eval,
                    .files = Vector_new(allocator),
            },
            .types = types,
            .symbols = symbols,
            .out = out,
    };
    Interpreter *interpreter = &_interpreter;

    InterpreterFileRef mainFile = Interpreter_load(interpreter, fs_in, inputFilePath);
    Interpreter_eval(interpreter, mainFile);

    if (flags.run) {
        if (flags.print_run) {
            fprintf_s(out, STR("RUN:\n---\n"));
        }
        Symbol entry;
        bool hasMain = Symbols_lookup(symbols, STR("main"), &entry);
        (void) hasMain;
        assert(hasMain && Types_lookup(types, entry.type)->kind == Type_Function && "main is a function");
        func_call(interpreter, entry.value, (Slice(Value)) {._begin = NULL, ._end = NULL,}, (InterpreterFileNodeRef) {.file = {0}, .node = {0}});
    } else {
        if (flags.print_emit) {
            fprintf_s(out, STR("EMIT:\n----\n"));
        }
        Buffer outBuf = Buffer_new(allocator);
        File *f = flags.buffer ? Buffer_asFile(allocator, &outBuf) : outputFile;
        emit_output ret = do_emit((emit_input) {
                .interpreter = interpreter,
                .target = target(targetName),
        });
        Vector_loop(compile_file, &ret.files, i) {
            compile_file *it = Vector_at(&ret.files, i);
            if (!String_equals(it->ext, STR("h"))) continue; // FIXME
            emit(interpreter, f, it);
        }
        Vector_loop(compile_file, &ret.files, i) {
            compile_file *it = Vector_at(&ret.files, i);
            if (!String_equals(it->ext, STR("c"))) continue; // FIXME
            emit(interpreter, f, it);
        }
        Vector_delete(compile_file, &ret.files);
        if (flags.buffer) {
            fs_close(f);
            fprintf_raw(outputFile, Buffer_toSlice(&outBuf));
        }
        if (outputFile != out) {
            FilePath_delete(&outputFilePath);
            fs_close(outputFile);
        }
    }

    fprintf_s(out, STR("\n"));
    Vector_delete(InterpreterFilePtr, &interpreter->compilation.files);
    Vector_delete(Type, &interpreter->types->all);
    Vector_delete(SymbolScope, &interpreter->symbols->scopes);
    FileSystem_delete(fs_out);
    FileSystem_delete(fs_in);
    fs_close(out);
    assert(!debugAllocator.size && "no memory leaked");
    return EXIT_SUCCESS;
}

static void emit(Interpreter *interpreter, File *f, compile_file *it)
{
    Allocator *allocator = interpreter->allocator;
    fprintf_s(f, STR("// file://"));
    Buffer buf = Buffer_new(allocator);
    fs_path_to_native(allocator, interpreter->fs_in->root, &buf);
    String slash = STR("/");
    Vector_push(&buf, *Slice_at(&slash.bytes, 0));
    const InterpreterFile *infile = Interpreter_lookup_file(interpreter, it->file);
    fs_path_to_native(allocator, infile->path, &buf);
    String dot = STR(".");
    Vector_push(&buf, *Slice_at(&dot.bytes, 0));
    _Vector_push(sizeof(uint8_t), &buf, String_sizeBytes(it->ext), String_begin(it->ext), String_sizeBytes(it->ext));
    fprintf_s(f, String_fromSlice(Buffer_toSlice(&buf), ENCODING_DEFAULT));
    Buffer_delete(&buf);
    fprintf_s(f, STR("\n\n"));
    fprintf_raw(f, Buffer_toSlice(it->content));
    fprintf_s(f, STR("\n"));
}
