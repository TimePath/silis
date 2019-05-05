#include <system.h>
#include "js.h"

#include <lib/stdio.h>

#include <compiler/type.h>
#include <compiler/types.h>

static const bool types = false;

static void tgt_js_file_begin(struct Target *target, Env env, compilation_file_ref file_ref, Vector(compile_file) *files);

static void tgt_js_file_end(struct Target *target, Env env, const compile_file *file);

static void tgt_js_func_forward(struct Target *target, Env env, const compile_file *file, type_id T, String name);

static void tgt_js_func_declare(struct Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[]);

static void tgt_js_var_begin(struct Target *target, Env env, const compile_file *file, type_id T);

static void tgt_js_var_end(struct Target *target, Env env, const compile_file *file, type_id T);

static void tgt_js_identifier(struct Target *target, Env env, const compile_file *file, String name);

Target target_js = {
        ._file_begin = tgt_js_file_begin,
        ._file_end = tgt_js_file_end,
        ._func_forward = tgt_js_func_forward,
        ._func_declare = tgt_js_func_declare,
        ._var_begin = tgt_js_var_begin,
        ._var_end = tgt_js_var_end,
        ._identifier = tgt_js_identifier,
};

typedef struct {
    bool local;
    bool anonymous;
} tgt_js_print_decl_opts;

static void tgt_js_print_decl_pre(struct Target *target, Env env, const compile_file *file, type_id T, tgt_js_print_decl_opts opts);

static void tgt_js_print_decl_post(struct Target *target, Env env, const compile_file *file, type_id T, const String idents[], tgt_js_print_decl_opts opts);

static void tgt_js_print_function(struct Target *target, Env env, const compile_file *file, type_id T, String ident, const String idents[]);

static void tgt_js_file_begin(struct Target *target, Env env, compilation_file_ref file_ref, Vector(compile_file) *files)
{
    (void) target;
    Allocator *allocator = env.allocator;
    compile_file _file = compile_file_new(allocator, file_ref, types ? STR("ts") : STR("js"), 0);
    Vector_push(files, _file);
}

static void tgt_js_file_end(struct Target *target, Env env, const compile_file *file)
{
    (void) target;
    (void) env;
    (void) file;
}

static void tgt_js_func_forward(struct Target *target, Env env, const compile_file *file, type_id T, String name)
{
    (void) target;
    (void) env;
    (void) file;
    (void) T;
    (void) name;
}

static void tgt_js_func_declare(struct Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[])
{
    tgt_js_print_function(target, env, file, T, name, argnames);
}

static void tgt_js_var_begin(struct Target *target, Env env, const compile_file *file, type_id T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_js_print_decl_pre(target, env, file, T, opts);
}

static void tgt_js_var_end(struct Target *target, Env env, const compile_file *file, type_id T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    if (!types) {
        fprintf_s(file->out, STR(" /*"));
    }
    tgt_js_print_decl_post(target, env, file, T, NULL, opts);
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}

static void tgt_js_identifier(struct Target *target, Env env, const compile_file *file, String name)
{
    (void) target;
    (void) env;
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

static void tgt_js_print_function(struct Target *target, Env env, const compile_file *file, type_id T, String ident, const String idents[])
{
    tgt_js_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_js_print_decl_pre(target, env, file, T, opts);
    if (!opts.anonymous) {
        tgt_js_identifier(target, env, file, ident);
    }
    tgt_js_print_decl_post(target, env, file, T, idents, opts);
}

static void tgt_js_print_type(struct Target *target, Env env, const compile_file *file, type_id T)
{
#define CASE(t) if (T.value == env.types->t.value)
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
    const type_t *type = type_lookup(env.types, T);
    if (type->kind == TYPE_FUNCTION) {
        fprintf_s(file->out, STR("("));
        type_id argT;
        const type_t *argType = type;
        while (true) {
            tgt_js_print_type(target, env, file, argType->u.func.in);
            argT = argType->u.func.out;
            argType = type_lookup(env.types, argT);
            if (argType->kind != TYPE_FUNCTION) {
                break;
            }
            fprintf_s(file->out, STR(", "));
        }
        fprintf_s(file->out, STR(") => "));
        tgt_js_print_type(target, env, file, argT);
        return;
    }
    fprintf_s(file->out, STR("type<"));
    fprintf_zu(file->out, T.value);
    fprintf_s(file->out, STR(">"));
}

static void tgt_js_print_decl_pre(struct Target *target, Env env, const compile_file *file, type_id T, tgt_js_print_decl_opts opts)
{
    (void) target;
    (void) opts;
    const type_t *type = type_lookup(env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(file->out, STR("let "));
        return;
    }
    fprintf_s(file->out, STR("function "));
}

static void tgt_js_print_decl_post(struct Target *target, Env env, const compile_file *file, type_id T, const String idents[], tgt_js_print_decl_opts opts)
{
    (void) opts;
    const type_t *type = type_lookup(env.types, T);
    if (type->kind != TYPE_FUNCTION || opts.local) {
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(target, env, file, T);
        return;
    }
    fprintf_s(file->out, STR("("));
    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        if (arg.value == env.types->t_unit.value) {
            break;
        }
        const String s = idents ? idents[i++] : STR("");
        tgt_js_identifier(target, env, file, s);
        if (!types) {
            fprintf_s(file->out, STR(" /*"));
        }
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(target, env, file, arg);
        if (!types) {
            fprintf_s(file->out, STR(" */"));
        }
        const type_t *next = type_lookup(env.types, argp->u.func.out);
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
    tgt_js_print_type(target, env, file, type_func_ret(env.types, type));
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}
