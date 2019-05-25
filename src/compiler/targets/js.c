#include <system.h>
#include "js.h"

#include <lib/stdio.h>

#include <interpreter/type.h>
#include <interpreter/types.h>

static const bool types = false;

static void tgt_js_file_begin(struct Target *target, Interpreter *interpreter, InterpreterFileRef file_ref, Vector(compile_file) *files);

static void tgt_js_file_end(struct Target *target, Interpreter *interpreter, const compile_file *file);

static void tgt_js_func_forward(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String name);

static void tgt_js_func_declare(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String name, const String argnames[]);

static void tgt_js_var_begin(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T);

static void tgt_js_var_end(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T);

static void tgt_js_identifier(struct Target *target, Interpreter *interpreter, const compile_file *file, String name);

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

static void tgt_js_print_decl_pre(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, tgt_js_print_decl_opts opts);

static void tgt_js_print_decl_post(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, const String idents[], tgt_js_print_decl_opts opts);

static void tgt_js_print_function(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String ident, const String idents[]);

static void tgt_js_file_begin(struct Target *target, Interpreter *interpreter, InterpreterFileRef file_ref, Vector(compile_file) *files)
{
    (void) target;
    Allocator *allocator = interpreter->allocator;
    compile_file _file = compile_file_new(allocator, file_ref, types ? STR("ts") : STR("js"), 0);
    Vector_push(files, _file);
}

static void tgt_js_file_end(struct Target *target, Interpreter *interpreter, const compile_file *file)
{
    (void) target;
    (void) interpreter;
    (void) file;
}

static void tgt_js_func_forward(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String name)
{
    (void) target;
    (void) interpreter;
    (void) file;
    (void) T;
    (void) name;
}

static void tgt_js_func_declare(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String name, const String argnames[])
{
    tgt_js_print_function(target, interpreter, file, T, name, argnames);
}

static void tgt_js_var_begin(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    tgt_js_print_decl_pre(target, interpreter, file, T, opts);
}

static void tgt_js_var_end(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T)
{
    tgt_js_print_decl_opts opts = {
            .local = true,
            .anonymous = false,
    };
    if (!types) {
        fprintf_s(file->out, STR(" /*"));
    }
    tgt_js_print_decl_post(target, interpreter, file, T, NULL, opts);
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}

static void tgt_js_identifier(struct Target *target, Interpreter *interpreter, const compile_file *file, String name)
{
    (void) target;
    (void) interpreter;
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
        fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
        fprintf_s(file->out, replace);
        it = enc->next(it);
        begin = Slice_begin(&it);
    }
    fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
}

// implementation

static void tgt_js_print_function(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, String ident, const String idents[])
{
    tgt_js_print_decl_opts opts = {
            .local = false,
            .anonymous = !String_sizeBytes(ident),
    };
    tgt_js_print_decl_pre(target, interpreter, file, T, opts);
    if (!opts.anonymous) {
        tgt_js_identifier(target, interpreter, file, ident);
    }
    tgt_js_print_decl_post(target, interpreter, file, T, idents, opts);
}

static void tgt_js_print_type(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T)
{
#define CASE(t) if (T.value == interpreter->types->t.value)
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
    const Type *type = Types_lookup(interpreter->types, T);
    if (type->kind.val == Type_Function) {
        fprintf_s(file->out, STR("("));
        TypeRef argT;
        const Type *argType = type;
        while (true) {
            tgt_js_print_type(target, interpreter, file, argType->u.Function.in);
            argT = argType->u.Function.out;
            argType = Types_lookup(interpreter->types, argT);
            if (argType->kind.val != Type_Function) {
                break;
            }
            fprintf_s(file->out, STR(", "));
        }
        fprintf_s(file->out, STR(") => "));
        tgt_js_print_type(target, interpreter, file, argT);
        return;
    }
    fprintf_s(file->out, STR("type<"));
    fprintf_zu(file->out, T.value);
    fprintf_s(file->out, STR(">"));
}

static void tgt_js_print_decl_pre(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, tgt_js_print_decl_opts opts)
{
    (void) target;
    (void) opts;
    const Type *type = Types_lookup(interpreter->types, T);
    if (type->kind.val != Type_Function || opts.local) {
        fprintf_s(file->out, STR("let "));
        return;
    }
    fprintf_s(file->out, STR("function "));
}

static void tgt_js_print_decl_post(struct Target *target, Interpreter *interpreter, const compile_file *file, TypeRef T, const String idents[], tgt_js_print_decl_opts opts)
{
    (void) opts;
    const Type *type = Types_lookup(interpreter->types, T);
    if (type->kind.val != Type_Function || opts.local) {
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(target, interpreter, file, T);
        return;
    }
    fprintf_s(file->out, STR("("));
    const Type *argp = type;
    size_t i = 0;
    while (true) {
        const TypeRef arg = argp->u.Function.in;
        if (arg.value == interpreter->types->t_unit.value) {
            break;
        }
        const String s = idents ? idents[i++] : STR("");
        tgt_js_identifier(target, interpreter, file, s);
        if (!types) {
            fprintf_s(file->out, STR(" /*"));
        }
        fprintf_s(file->out, STR(": "));
        tgt_js_print_type(target, interpreter, file, arg);
        if (!types) {
            fprintf_s(file->out, STR(" */"));
        }
        const Type *next = Types_lookup(interpreter->types, argp->u.Function.out);
        if (next->kind.val != Type_Function) {
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
    tgt_js_print_type(target, interpreter, file, Types_function_result(interpreter->types, T));
    if (!types) {
        fprintf_s(file->out, STR(" */"));
    }
}
