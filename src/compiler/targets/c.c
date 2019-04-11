#include <system.h>
#include "c.h"

#include <lib/stdio.h>

#include <compiler/type.h>
#include <compiler/types.h>

static void tgt_c_file_begin(struct Target *target, Env env, const compile_file *file);

static void tgt_c_file_end(struct Target *target, Env env, const compile_file *file);

static void tgt_c_func_forward(struct Target *target, Env env, const compile_file *file, type_id T, String name);

static void tgt_c_func_declare(struct Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[]);

static void tgt_c_var_begin(struct Target *target, Env env, const compile_file *file, type_id T);

static void tgt_c_var_end(struct Target *target, Env env, const compile_file *file, type_id T);

static void tgt_c_identifier(struct Target *target, Env env, const compile_file *file, String name);

Target target_c = {
        .file_begin = tgt_c_file_begin,
        .file_end = tgt_c_file_end,
        .func_forward = tgt_c_func_forward,
        .func_declare = tgt_c_func_declare,
        .var_begin = tgt_c_var_begin,
        .var_end = tgt_c_var_end,
        .identifier = tgt_c_identifier,
};

typedef struct {
    bool local;
    bool anonymous;
} tgt_c_print_decl_opts;

static void tgt_c_print_decl_pre(struct Target *target, Env env, const compile_file *file, type_id T, tgt_c_print_decl_opts opts);

static void tgt_c_print_decl_post(struct Target *target, Env env, const compile_file *file, type_id T, const String idents[], tgt_c_print_decl_opts opts);

static void tgt_c_print_function(struct Target *target, Env env, const compile_file *file, type_id T, String ident, const String idents[]);

static void tgt_c_file_begin(struct Target *target, Env env, const compile_file *file)
{
    (void) target;
    (void) env;
    fprintf_s(file->out, STR("typedef const char *string;\n"));
}

static void tgt_c_file_end(struct Target *target, Env env, const compile_file *file)
{
    (void) target;
    (void) env;
    (void) file;
}

static void tgt_c_func_forward(struct Target *target, Env env, const compile_file *file, type_id T, String name)
{
    tgt_c_print_function(target, env, file, T, name, NULL);
    fprintf_s(file->out, STR(";"));
}

static void tgt_c_func_declare(struct Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[])
{
    tgt_c_print_function(target, env, file, T, name, argnames);
}

static void tgt_c_var_begin(struct Target *target, Env env, const compile_file *file, type_id T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_pre(target, env, file, T, opts);
}

static void tgt_c_var_end(struct Target *target, Env env, const compile_file *file, type_id T)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_c_print_decl_post(target, env, file, T, NULL, opts);
}

static void tgt_c_identifier(struct Target *target, Env env, const compile_file *file, String name)
{
    (void) target;
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

static void tgt_c_print_function(struct Target *target, Env env, const compile_file *file, type_id T, String ident, const String idents[])
{
    tgt_c_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(target, env, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(target, env, file, ident);
    }
    tgt_c_print_decl_post(target, env, file, T, idents, opts);
}

static void tgt_c_print_declaration(struct Target *target, Env env, const compile_file *file, type_id T, String ident)
{
    tgt_c_print_decl_opts opts = {
            .local = true,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_c_print_decl_pre(target, env, file, T, opts);
    if (!opts.anonymous) {
        tgt_c_identifier(target, env, file, ident);
    }
    tgt_c_print_decl_post(target, env, file, T, NULL, opts);
}

static void tgt_c_print_decl_pre(struct Target *target, Env env, const compile_file *file, type_id T, tgt_c_print_decl_opts opts)
{
    const type_t *type = type_lookup(env.types, T);
    if (type->kind != TYPE_FUNCTION) {
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
    tgt_c_print_declaration(target, env, file, type_func_ret(env.types, type), STR(""));
    if (!opts.anonymous) {
        fprintf_s(file->out, STR(" "));
    }
    if (opts.local) {
        fprintf_s(file->out, STR("(*"));
    }
}

static void tgt_c_print_decl_post(struct Target *target, Env env, const compile_file *file, type_id T, const String idents[], tgt_c_print_decl_opts opts)
{
    const type_t *type = type_lookup(env.types, T);
    if (type->kind != TYPE_FUNCTION) {
        return;
    }
    if (opts.local) {
        fprintf_s(file->out, STR(")"));
    }
    fprintf_s(file->out, STR("("));
    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        const String s = idents ? idents[i++] : STR("");
        tgt_c_print_declaration(target, env, file, arg, s);
        const type_t *next = type_lookup(env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        fprintf_s(file->out, STR(", "));
    }
    fprintf_s(file->out, STR(")"));
}
