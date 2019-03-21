#include <system.h>
#include "js.h"

#include <lib/stdio.h>

#include <compiler/type.h>
#include <compiler/types.h>

static const bool types = false;

static void tgt_js_file_begin(const compile_ctx_t *ctx, const compile_file *file);

static void tgt_js_file_end(const compile_ctx_t *ctx, const compile_file *file);

static void tgt_js_func_forward(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name);

static void tgt_js_func_declare(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name, const String argnames[]);

static void tgt_js_var_begin(const compile_ctx_t *ctx, const compile_file *file, type_id T);

static void tgt_js_var_end(const compile_ctx_t *ctx, const compile_file *file, type_id T);

static void tgt_js_identifier(const compile_ctx_t *ctx, const compile_file *file, String name);

Target target_js = {
        .file_begin = tgt_js_file_begin,
        .file_end = tgt_js_file_end,
        .func_forward = tgt_js_func_forward,
        .func_declare = tgt_js_func_declare,
        .var_begin = tgt_js_var_begin,
        .var_end = tgt_js_var_end,
        .identifier = tgt_js_identifier,
};

typedef struct {
    bool local;
    bool anonymous;
} tgt_js_print_decl_opts;

static void tgt_js_print_decl_pre(const compile_ctx_t *ctx, const compile_file *file, type_id T, tgt_js_print_decl_opts opts);

static void tgt_js_print_decl_post(const compile_ctx_t *ctx, const compile_file *file, type_id T, const String idents[], tgt_js_print_decl_opts opts);

static void tgt_js_print_function(const compile_ctx_t *ctx, const compile_file *file, type_id T, String ident, const String idents[]);

static void tgt_js_file_begin(const compile_ctx_t *ctx, const compile_file *file)
{
    (void) ctx;
    (void) file;
}

static void tgt_js_file_end(const compile_ctx_t *ctx, const compile_file *file)
{
    (void) ctx;
    (void) file;
}

static void tgt_js_func_forward(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name)
{
    (void) ctx;
    (void) file;
    (void) T;
    (void) name;
}

static void tgt_js_func_declare(const compile_ctx_t *ctx, const compile_file *file, type_id T, String name, const String argnames[])
{
    tgt_js_print_function(ctx, file, T, name, argnames);
}

static void tgt_js_var_begin(const compile_ctx_t *ctx, const compile_file *file, type_id T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_js_print_decl_pre(ctx, file, T, opts);
}

static void tgt_js_var_end(const compile_ctx_t *ctx, const compile_file *file, type_id T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    if (!types) {
        fprintf_s(file->out, STR(" /*"));
    }
    tgt_js_print_decl_post(ctx, file, T, NULL, opts);
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}

static void tgt_js_identifier(const compile_ctx_t *ctx, const compile_file *file, String name)
{
    (void) ctx;
    const StringEncoding *enc = name.encoding;
    const uint8_t *begin = String_begin(name);
    Slice(uint8_t) it = name.bytes;
    for (; Slice_begin(&it) != Slice_end(&it);) {
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

static void tgt_js_print_function(const compile_ctx_t *ctx, const compile_file *file, type_id T, String ident, const String idents[])
{
    tgt_js_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_js_print_decl_pre(ctx, file, T, opts);
    if (!opts.anonymous) {
        tgt_js_identifier(ctx, file, ident);
    }
    tgt_js_print_decl_post(ctx, file, T, idents, opts);
}

static void tgt_js_print_type(const compile_ctx_t *ctx, const compile_file *file, type_id T)
{
#define CASE(t) if (T.value == ctx->env.types->t.value)
    CASE(t_unit) {
        fprintf_s(file->out, STR("void"));
        return;
    }
    CASE(t_int) {
        fprintf_s(file->out, STR("number"));
        return;
    }
    CASE(t_string) {
        fprintf_s(file->out, STR("string"));
        return;
    }
#undef CASE
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind == TYPE_FUNCTION) {
        fprintf_s(file->out, STR("("));
        type_id argT;
        const type_t *argType = type;
        while (true) {
            tgt_js_print_type(ctx, file, argType->u.func.in);
            argT = argType->u.func.out;
            argType = type_lookup(ctx->env.types, argT);
            if (argType->kind != TYPE_FUNCTION) {
                break;
            }
            fprintf_s(file->out, STR(", "));
        }
        fprintf_s(file->out, STR(") => "));
        tgt_js_print_type(ctx, file, argT);
        return;
    }
    fprintf_s(file->out, STR("type<"));
    fprintf_zu(file->out, T.value);
    fprintf_s(file->out, STR(">"));
}

static void tgt_js_print_decl_pre(const compile_ctx_t *ctx, const compile_file *file, type_id T, tgt_js_print_decl_opts opts)
{
    (void) opts;
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(file->out, STR("let "));
        return;
    }
    fprintf_s(file->out, STR("function "));
}

static void tgt_js_print_decl_post(const compile_ctx_t *ctx, const compile_file *file, type_id T, const String idents[], tgt_js_print_decl_opts opts)
{
    (void) opts;
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(ctx, file, T);
        return;
    }
    fprintf_s(file->out, STR("("));
    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        if (arg.value == ctx->env.types->t_unit.value) {
            break;
        }
        const String s = idents ? idents[i++] : STR("");
        tgt_js_identifier(ctx, file, s);
        if (!types) {
            fprintf_s(file->out, STR(" /*"));
        }
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(ctx, file, arg);
        if (!types) {
            fprintf_s(file->out, STR(" */"));
        }
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        fprintf_s(file->out, STR(", "));
    }
    fprintf_s(file->out, STR(")"));
    if (!types) {
        fprintf_s(file->out, STR(" /*"));
    }
    fprintf_s(file->out, STR(": "));
    tgt_js_print_type(ctx, file, type_func_ret(ctx->env.types, type));
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}
