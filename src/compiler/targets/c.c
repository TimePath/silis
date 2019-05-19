#include <system.h>
#include "c.h"

#include <lib/stdio.h>

#include <interpreter/type.h>
#include <interpreter/types.h>

static void tgt_c_file_begin(Target *self, Env env, compilation_file_ref file_ref, Vector(compile_file) *files);

static void tgt_c_file_end(Target *self, Env env, const compile_file *file);

static void tgt_c_func_forward(Target *self, Env env, const compile_file *file, TypeRef T, String name);

static void tgt_c_func_declare(Target *self, Env env, const compile_file *file, TypeRef T, String name, const String argnames[]);

static void tgt_c_var_begin(Target *self, Env env, const compile_file *file, TypeRef T);

static void tgt_c_var_end(Target *self, Env env, const compile_file *file, TypeRef T);

static void tgt_c_identifier(Target *self, Env env, const compile_file *file, String name);

Target target_c = {
        ._file_begin = tgt_c_file_begin,
        ._file_end = tgt_c_file_end,
        ._func_forward = tgt_c_func_forward,
        ._func_declare = tgt_c_func_declare,
        ._var_begin = tgt_c_var_begin,
        ._var_end = tgt_c_var_end,
        ._identifier = tgt_c_identifier,
};

static const size_t FLAG_HEADER = 1 << 0;

typedef struct {
    bool local;
    bool anonymous;
} tgt_c_print_decl_opts;

static void tgt_c_print_decl_pre(Target *self, Env env, const compile_file *file, TypeRef T, tgt_c_print_decl_opts opts);

static void tgt_c_print_decl_post(Target *self, Env env, const compile_file *file, TypeRef T, const String idents[], tgt_c_print_decl_opts opts);

static void tgt_c_print_function(Target *self, Env env, const compile_file *file, TypeRef T, String ident, const String idents[]);

static void tgt_c_file_begin(Target *self, Env env, compilation_file_ref file_ref, Vector(compile_file) *files)
{
    (void) self;
    Allocator *allocator = env.allocator;
    compile_file _header = compile_file_new(allocator, file_ref, STR("h"), FLAG_HEADER);
    compile_file *header = &_header;
    fprintf_s(header->out, STR("#pragma once\n"));
    fprintf_s(header->out, STR("typedef const char *string;\n"));
    Vector_push(files, _header);

    compile_file _impl = compile_file_new(allocator, file_ref, STR("c"), 0);
    Vector_push(files, _impl);
}

static void tgt_c_file_end(Target *self, Env env, const compile_file *file)
{
    (void) self;
    (void) env;
    (void) file;
}

static void tgt_c_func_forward(Target *self, Env env, const compile_file *file, TypeRef T, String name)
{
    if (!(file->flags & FLAG_HEADER)) {
        return;
    }
    tgt_c_print_function(self, env, file, T, name, NULL);
    fprintf_s(file->out, STR(";"));
}

static void tgt_c_func_declare(Target *self, Env env, const compile_file *file, TypeRef T, String name, const String argnames[])
{
    tgt_c_print_function(self, env, file, T, name, argnames);
}

static void tgt_c_var_begin(Target *self, Env env, const compile_file *file, TypeRef T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_pre(self, env, file, T, opts);
}

static void tgt_c_var_end(Target *self, Env env, const compile_file *file, TypeRef T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_post(self, env, file, T, NULL, opts);
}

static void tgt_c_identifier(Target *self, Env env, const compile_file *file, String name)
{
    (void) self;
    (void) env;
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
        fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
        fprintf_s(file->out, replace);
        it = enc->next(it);
        begin = it._begin;
    }
    fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
}

// implementation

static void tgt_c_print_function(Target *self, Env env, const compile_file *file, TypeRef T, String ident, const String idents[])
{
    tgt_c_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(self, env, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(self, env, file, ident);
    }
    tgt_c_print_decl_post(self, env, file, T, idents, opts);
}

static void tgt_c_print_declaration(Target *self, Env env, const compile_file *file, TypeRef T, String ident)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(self, env, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(self, env, file, ident);
    }
    tgt_c_print_decl_post(self, env, file, T, NULL, opts);
}

static void tgt_c_print_decl_pre(Target *self, Env env, const compile_file *file, TypeRef T, tgt_c_print_decl_opts opts)
{
    const Type *type = Types_lookup(env.types, T);
    if (type->kind != Type_Function) {
#define CASE(t) if (T.value == env.types->t.value)
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
        fprintf_zu(file->out, T.value);
        fprintf_s(file->out, !opts.anonymous ? STR("> ") : STR(">"));
        return;
    }
    tgt_c_print_declaration(self, env, file, Types_function_result(env.types, T), STR(""));
    if (!opts.anonymous) {
        fprintf_s(file->out, STR(" "));
    }
    if (opts.local) {
        fprintf_s(file->out, STR("(*"));
    }
}

static void tgt_c_print_decl_post(Target *self, Env env, const compile_file *file, TypeRef T, const String idents[], tgt_c_print_decl_opts opts)
{
    const Type *type = Types_lookup(env.types, T);
    if (type->kind != Type_Function) {
        return;
    }
    if (opts.local) {
        fprintf_s(file->out, STR(")"));
    }
    fprintf_s(file->out, STR("("));
    const Type *argp = type;
    size_t i = 0;
    while (true) {
        const TypeRef arg = argp->u.Function.in;
        const String s = idents ? idents[i++] : STR("");
        tgt_c_print_declaration(self, env, file, arg, s);
        const Type *next = Types_lookup(env.types, argp->u.Function.out);
        if (next->kind != Type_Function) {
            break;
        }
        argp = next;
        fprintf_s(file->out, STR(", "));
    }
    fprintf_s(file->out, STR(")"));
}
