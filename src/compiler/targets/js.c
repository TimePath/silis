#include <system.h>
#include "js.h"

#include <lib/stdio.h>

#include <compiler/type.h>
#include <compiler/types.h>

static void _file_begin(const compile_ctx_t *ctx);

static void _file_end(const compile_ctx_t *ctx);

static void _func_forward(const compile_ctx_t *ctx, type_id T, String name);

static void _func_declare(const compile_ctx_t *ctx, type_id T, String name, const String argnames[]);

static void _var_begin(const compile_ctx_t *ctx, type_id T);

static void _var_end(const compile_ctx_t *ctx, type_id T);

Target target_js = {
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
    (void) ctx;
}

static void _file_end(const compile_ctx_t *ctx)
{
    (void) ctx;
}

static void _func_forward(const compile_ctx_t *ctx, type_id T, String name)
{
    (void) ctx;
    (void) T;
    (void) name;
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
    fprintf_s(ctx->out, STR(" /*"));
    print_decl_post(ctx, T, NULL, opts);
    fprintf_s(ctx->out, STR(" */"));
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

static void print_type(const compile_ctx_t *ctx, type_id T)
{
#define CASE(t) if (T.value == ctx->env.types->t.value)
    CASE(t_unit) {
        fprintf_s(ctx->out, STR("void"));
        return;
    }
    CASE(t_int) {
        fprintf_s(ctx->out, STR("number"));
        return;
    }
    CASE(t_string) {
        fprintf_s(ctx->out, STR("string"));
        return;
    }
#undef CASE
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind == TYPE_FUNCTION) {
        fprintf_s(ctx->out, STR("("));
        type_id argT;
        const type_t *argType = type;
        while (true) {
            print_type(ctx, argType->u.func.in);
            argT = argType->u.func.out;
            argType = type_lookup(ctx->env.types, argT);
            if (argType->kind != TYPE_FUNCTION) {
                break;
            }
            fprintf_s(ctx->out, STR(", "));
        }
        fprintf_s(ctx->out, STR(") => "));
        print_type(ctx, argT);
        return;
    }
    fprintf_s(ctx->out, STR("type<"));
    fprintf_zu(ctx->out, T.value);
    fprintf_s(ctx->out, STR(">"));
}

static void print_decl_pre(const compile_ctx_t *ctx, type_id T, print_decl_opts opts)
{
    (void) opts;
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(ctx->out, STR("let "));
        return;
    }
    fprintf_s(ctx->out, STR("function "));
}

static void print_decl_post(const compile_ctx_t *ctx, type_id T, const String idents[], print_decl_opts opts)
{
    (void) opts;
    const type_t *type = type_lookup(ctx->env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(ctx->out, STR(": "));
        print_type(ctx, T);
        return;
    }
    fprintf_s(ctx->out, STR("("));
    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        if (arg.value == ctx->env.types->t_unit.value) {
            break;
        }
        const String s = idents ? idents[i++] : STR("");
        fprintf_s(ctx->out, s);
        fprintf_s(ctx->out, STR(" /*"));
        fprintf_s(ctx->out, STR(": "));
        print_type(ctx, arg);
        fprintf_s(ctx->out, STR(" */"));
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        fprintf_s(ctx->out, STR(", "));
    }
    fprintf_s(ctx->out, STR(")"));
    fprintf_s(ctx->out, STR(" /*"));
    fprintf_s(ctx->out, STR(": "));
    print_type(ctx, type_func_ret(ctx->env.types, type));
    fprintf_s(ctx->out, STR(" */"));
}
