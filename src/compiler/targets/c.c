#include <prelude.h>
#include "c.h"

#include <lib/stdio.h>

#include <interpreter/type.h>
#include <interpreter/types.h>

static void tgt_c_file_begin(Target *self, Interpreter *interpreter, Ref(InterpreterFilePtr) file_ref, Vector(compile_file) *files);

static void tgt_c_file_end(Target *self, Interpreter *interpreter, const compile_file *file);

static void tgt_c_func_forward(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name);

static void tgt_c_func_declare(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name, Slice(String) argnames);

static void tgt_c_var_begin(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T);

static void tgt_c_var_end(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T);

static void tgt_c_identifier(Target *self, Interpreter *interpreter, const compile_file *file, String name);

Target target_c = {
        ._file_begin = tgt_c_file_begin,
        ._file_end = tgt_c_file_end,
        ._func_forward = tgt_c_func_forward,
        ._func_declare = tgt_c_func_declare,
        ._var_begin = tgt_c_var_begin,
        ._var_end = tgt_c_var_end,
        ._identifier = tgt_c_identifier,
};

enum {
    STAGE_HEADER,
    STAGE_IMPL,
};

typedef struct {
    bool local;
    bool anonymous;
} tgt_c_print_decl_opts;

static void tgt_c_print_decl_pre(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, tgt_c_print_decl_opts opts);

static void tgt_c_print_decl_post(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, Slice(String) idents, tgt_c_print_decl_opts opts);

static void tgt_c_print_function(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String ident, Slice(String) idents);

static void tgt_c_file_begin(Target *self, Interpreter *interpreter, Ref(InterpreterFilePtr) file_ref, Vector(compile_file) *files)
{
    (void) self;
    Allocator *allocator = interpreter->allocator;
    compile_file _header = compile_file_new(file_ref, STR("h"), STAGE_HEADER, Slice_of(file_flag, ((Array(file_flag, 1)) { FLAG_HEADER })), allocator);
    compile_file *header = &_header;
    fprintf_s(header->out, STR("#pragma once\n"));
    fprintf_s(header->out, STR("typedef const char *string;\n"));
    Vector_push(files, _header);

    compile_file _impl = compile_file_new(file_ref, STR("c"), STAGE_IMPL, Slice_of(file_flag, ((Array(file_flag, 1)) { FLAG_IMPL })), allocator);
    compile_file *impl = &_impl;
    fprintf_s(impl->out, STR("#include \"./"));
    Buffer buf = Buffer_new(allocator);
    const InterpreterFile *infile = Interpreter_lookup_file(interpreter, file_ref);
    FilePath basename = FilePath_basename(infile->path, allocator);
    FilePath_to_native(basename, &buf);
    FilePath_delete(&basename);
    fprintf_s(impl->out, String_fromSlice(Buffer_toSlice(&buf), ENCODING_DEFAULT));
    Buffer_delete(&buf);
    fprintf_s(impl->out, STR(".h\"\n"));
    Vector_push(files, _impl);
}

static void tgt_c_file_end(Target *self, Interpreter *interpreter, const compile_file *file)
{
    (void) self;
    (void) interpreter;
    (void) file;
}

static void tgt_c_func_forward(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name)
{
    if (!(file->flags & (1 << FLAG_HEADER))) {
        return;
    }
    tgt_c_print_function(self, interpreter, file, T, name, Slice_of_n(String, NULL, 0));
    fprintf_s(file->out, STR(";"));
}

static void tgt_c_func_declare(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name, Slice(String) argnames)
{
    tgt_c_print_function(self, interpreter, file, T, name, argnames);
}

static void tgt_c_var_begin(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_pre(self, interpreter, file, T, opts);
}

static void tgt_c_var_end(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_post(self, interpreter, file, T, Slice_of_n(String, NULL, 0), opts);
}

static void tgt_c_identifier(Target *self, Interpreter *interpreter, const compile_file *file, String name)
{
    (void) self;
    (void) interpreter;
    const StringEncoding *enc = name.encoding;
    const uint8_t *begin = String_begin(name);
    Slice(uint8_t) it = name.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); ) {
        size_t c = enc->get(it);
        String replace;
        switch (c) {
            default:
                it = enc->next(it);
                continue;
#define X(c, s) case c: replace = STR(s); break;
            X('.', "__dot__")
#undef X
        }
        fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
        fprintf_s(file->out, replace);
        it = enc->next(it);
        begin = Slice_begin(&it);
    }
    fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
}

// implementation

static void tgt_c_print_function(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String ident, Slice(String) idents)
{
    tgt_c_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(self, interpreter, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(self, interpreter, file, ident);
    }
    tgt_c_print_decl_post(self, interpreter, file, T, idents, opts);
}

static void tgt_c_print_declaration(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String ident)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(self, interpreter, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(self, interpreter, file, ident);
    }
    tgt_c_print_decl_post(self, interpreter, file, T, Slice_of_n(String, NULL, 0), opts);
}

static void tgt_c_print_decl_pre(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, tgt_c_print_decl_opts opts)
{
    const Type *type = Types_lookup(interpreter->types, T);
    if (type->kind.val != Type_Function) {
#define CASE(t) if (Ref_eq(T, interpreter->types->t))
        CASE(t_unit) {
            fprintf_s(file->out, !opts.anonymous ? STR("void ") : STR("void"));
            return;
        }
        CASE(t_int) {
            fprintf_s(file->out, !opts.anonymous ? STR("int ") : STR("int"));
            return;
        }
        CASE(t_string) {
            fprintf_s(file->out, !opts.anonymous ? STR("string ") : STR("string"));
            return;
        }
#undef CASE
        fprintf_s(file->out, STR("type<"));
        fprintf_zu(file->out, Ref_value(T));
        fprintf_s(file->out, !opts.anonymous ? STR("> ") : STR(">"));
        return;
    }
    tgt_c_print_declaration(self, interpreter, file, Types_function_result(interpreter->types, T), STR(""));
    if (!opts.anonymous) {
        fprintf_s(file->out, STR(" "));
    }
    if (opts.local) {
        fprintf_s(file->out, STR("(*"));
    }
}

static void tgt_c_print_decl_post(Target *self, Interpreter *interpreter, const compile_file *file, Ref(Type) T, Slice(String) idents, tgt_c_print_decl_opts opts)
{
    const Type *type = Types_lookup(interpreter->types, T);
    if (type->kind.val != Type_Function) {
        return;
    }
    if (opts.local) {
        fprintf_s(file->out, STR(")"));
    }
    fprintf_s(file->out, STR("("));
    const Type *argp = type;
    size_t i = 0;
    while (true) {
        const Ref(Type) arg = argp->u.Function.in;
        const String s = i < Slice_size(&idents) ? *Slice_at(&idents, i++) : STR("");
        tgt_c_print_declaration(self, interpreter, file, arg, s);
        const Type *next = Types_lookup(interpreter->types, argp->u.Function.out);
        if (next->kind.val != Type_Function) {
            break;
        }
        argp = next;
        fprintf_s(file->out, STR(", "));
    }
    fprintf_s(file->out, STR(")"));
}
