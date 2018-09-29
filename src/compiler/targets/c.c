#include <system.h>
#include "c.h"

#include <lib/stdio.h>

#include <compiler/type.h>
#include <compiler/types.h>

static void _file_begin(const compile_ctx_t *ctx);

static void _file_end(const compile_ctx_t *ctx);

static void _func_forward(const compile_ctx_t *ctx, type_id T, String name);

static void _func_declare(const compile_ctx_t *ctx, type_id T, String name, const String argnames[]);

static void _var_begin(const compile_ctx_t *ctx, type_id T);

static void _var_end(const compile_ctx_t *ctx, type_id T);

Target target_c = {
        .file_begin = _file_begin,
        .file_end = _file_end,
        .func_forward = _func_forward,
        .func_declare = _func_declare,
        .var_begin = _var_begin,
        .var_end = _var_end,
};

typedef struct {
    bool local;
    bool anonymous;
} print_decl_opts;

static void print_decl_pre(const compile_ctx_t *ctx, type_id T, print_decl_opts opts);

static void print_decl_post(const compile_ctx_t *ctx, type_id T, const String idents[], print_decl_opts opts);

static void print_function(const compile_ctx_t *ctx, type_id T, String ident, const String idents[]);

static void _file_begin(const compile_ctx_t *ctx)
{
    fprintf_s(ctx->out, STR("typedef const char *string;\n"));
}

static void _file_end(const compile_ctx_t *ctx)
{
    (void) ctx;
}

static void _func_forward(const compile_ctx_t *ctx, type_id T, String name)
{
    print_function(ctx, T, name, NULL);
    fprintf_s(ctx->out, STR(";"));
}

static void _func_declare(const compile_ctx_t *ctx, type_id T, String name, const String argnames[])
{
    print_function(ctx, T, name, argnames);
}

static void _var_begin(const compile_ctx_t *ctx, type_id T)
{
    print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    print_decl_pre(ctx, T, opts);
}

static void _var_end(const compile_ctx_t *ctx, type_id T)
{
    print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    print_decl_post(ctx, T, NULL, opts);
}

// implementation

static void print_function(const compile_ctx_t *ctx, type_id T, String ident, const String idents[])
{
    print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    print_decl_pre(ctx, T, opts);
    if (!opts.anonymous) {
        fprintf_s(ctx->out, ident);
    }
    print_decl_post(ctx, T, idents, opts);
}

static void print_declaration(const compile_ctx_t *ctx, type_id T, String ident)
{
    print_decl_opts opts = {
            .local = true,
            .anonymous = !String_sizeBytes(ident),
    };
    print_decl_pre(ctx, T, opts);
    if (!opts.anonymous) {
        fprintf_s(ctx->out, ident);
    }
    print_decl_post(ctx, T, NULL, opts);
}

static void print_decl_pre(const compile_ctx_t *ctx, type_id T, print_decl_opts opts)
{
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION) {
#define CASE(t) if (T.value == ctx->env.types->t.value)
        CASE(t_unit) {
            fprintf_s(ctx->out, !opts.anonymous ? STR("void ") : STR("void"));
            return;
        }
        CASE(t_int) {
            fprintf_s(ctx->out, !opts.anonymous ? STR("int ") : STR("int"));
            return;
        }
        CASE(t_string) {
            fprintf_s(ctx->out, !opts.anonymous ? STR("string ") : STR("string"));
            return;
        }
#undef CASE
        fprintf_s(ctx->out, STR("type<"));
        fprintf_zu(ctx->out, T.value);
        fprintf_s(ctx->out, !opts.anonymous ? STR("> ") : STR(">"));
        return;
    }
    print_declaration(ctx, type_func_ret(ctx->env.types, type), STR(""));
    if (!opts.anonymous) {
        fprintf_s(ctx->out, STR(" "));
    }
    if (opts.local) {
        fprintf_s(ctx->out, STR("(*"));
    }
}

static void print_decl_post(const compile_ctx_t *ctx, type_id T, const String idents[], print_decl_opts opts)
{
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION) {
        return;
    }
    if (opts.local) {
        fprintf_s(ctx->out, STR(")"));
    }
    fprintf_s(ctx->out, STR("("));
    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        const String s = idents ? idents[i++] : STR("");
        print_declaration(ctx, arg, s);
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        fprintf_s(ctx->out, STR(", "));
    }
    fprintf_s(ctx->out, STR(")"));
}
