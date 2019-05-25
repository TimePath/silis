#include <system.h>
#include "emit.h"

#include <lib/stdio.h>

#include <interpreter/interpreter.h>
#include <interpreter/symbols.h>
#include <compiler/targets/_.h>
#include <interpreter/type.h>
#include <interpreter/value.h>

#include <interpreter/eval.h>

#include <compiler/intrinsics/define.h>
#include <compiler/intrinsics/do.h>
#include <compiler/intrinsics/func.h>
#include <compiler/intrinsics/if.h>
#include <compiler/intrinsics/set.h>
#include <compiler/intrinsics/untyped.h>
#include <compiler/intrinsics/while.h>

typedef struct {
    Interpreter *interpreter;
    struct Target *target;
    size_t depth;
} emit_ctx;

#define LINE(...) MACRO_BEGIN \
    ctx->depth = ctx->depth __VA_ARGS__; \
    fprintf_s(file->out, STR("\n")); \
    fprintf_s(file->out, String_indent(allocator, ctx->depth * 4)); \
MACRO_END

#define SEMI() fprintf_s(file->out, STR(";"))

typedef enum {
    RETURN_NO,
    RETURN_FUNC,
    RETURN_TEMPORARY,
    RETURN_NAMED,
} return_e;

typedef struct {
    return_e kind;
    PADDING(4)
    TypeRef type;
    union {
        struct {
            InterpreterFileNodeRef val;
        } temporary;
        struct {
            String val;
        } named;
    } u;
} return_t;

static void emit_value(emit_ctx *ctx, const compile_file *file, const Value *it);

static void visit_node(emit_ctx *ctx, const compile_file *file, return_t ret, InterpreterFileNodeRef it);

emit_output do_emit(emit_input in)
{
    Allocator *allocator = in.interpreter->allocator;
    Vector(compile_file) files = Vector_new(allocator);

    emit_ctx _ctx = {
            .interpreter = in.interpreter,
            .target = in.target,
            .depth = 0,
    };
    emit_ctx *ctx = &_ctx;

    Symbol entry;
    bool hasMain = Symbols_lookup(ctx->interpreter->symbols, STR("main"), &entry);
    (void) entry;
    (void) hasMain;
    assert(hasMain && Types_lookup(ctx->interpreter->types, entry.value.type)->kind.val == Type_Function && "main is a function");

    Vector_loop(InterpreterFilePtr, &in.interpreter->compilation.files, i) {
        Target_file_begin(ctx->target, ctx->interpreter, (InterpreterFileRef) {i + 1}, &files);
    }

    const SymbolScope *globals = Vector_at(&ctx->interpreter->symbols->scopes, 0);
    // first pass: emit function prototypes and globals
    Vector_loop(TrieEntry, &globals->t.entries, i) {
        const TrieEntry *e = Vector_at(&globals->t.entries, i);
        const TrieNode(Symbol) *n = Vector_at(&globals->t.nodes, e->value);
        const Symbol _it = n->value;
        const Symbol *it = &_it;
        const compile_file *file = Vector_at(&files, (it->file.id - 1) * 2 + 0); // FIXME
        if (it->value.flags.intrinsic) {
            continue;
        }
        const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
        const TypeRef type = it->type;
        if (it->value.flags.native) {
            fprintf_s(file->out, STR("extern ")); // FIXME
        }
        if (Types_lookup(ctx->interpreter->types, type)->kind.val == Type_Function) {
            Target_func_forward(ctx->target, ctx->interpreter, file, type, ident);
        } else {
            Target_var_begin(ctx->target, ctx->interpreter, file, type);
            fprintf_s(file->out, ident);
            Target_var_end(ctx->target, ctx->interpreter, file, type);
            if (!it->value.flags.native) {
                fprintf_s(file->out, STR(" = "));
                emit_value(ctx, file, &it->value);
            }
            SEMI();
        }
        LINE();
    }
    // second pass: emit functions
    Vector_loop(TrieEntry, &globals->t.entries, i) {
        const TrieEntry *e = Vector_at(&globals->t.entries, i);
        const TrieNode(Symbol) *n = Vector_at(&globals->t.nodes, e->value);
        const Symbol _it = n->value;
        const Symbol *it = &_it;
        const compile_file *file = Vector_at(&files, (it->file.id - 1) * 2 + 1); // FIXME
        if (it->value.flags.intrinsic || it->value.flags.native) {
            continue;
        }
        const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
        const TypeRef type = it->type;
        if (Types_lookup(ctx->interpreter->types, type)->kind.val != Type_Function) {
            continue;
        }
        assert(it->file.id && "global is user-defined");
        LINE();
        NodeList argv = NodeList_iterator(ctx->interpreter, it->value.u.func.arglist);
        size_t argc = argv._n;
        String *argnames = malloc(sizeof(String) * argc);
        func_args_names(ctx->interpreter, argv, argnames);
        Target_func_declare(ctx->target, ctx->interpreter, file, type, ident, argnames);
        free(argnames);
        LINE();
        fprintf_s(file->out, STR("{"));
        LINE(+1);
        InterpreterFileNodeRef body = it->value.u.func.value;
        visit_node(ctx, file, (return_t) {.kind = RETURN_FUNC}, body);
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        LINE();
    }

    Vector_loop(compile_file, &files, i) {
        const compile_file *file = Vector_at(&files, i);
        Target_file_end(ctx->target, ctx->interpreter, file);
    }

    return (emit_output) {
            .files = files,
    };
}

static void emit_atom(emit_ctx *ctx, const compile_file *file, String value);

static void emit_integral(emit_ctx *ctx, const compile_file *file, size_t value);

static void emit_string(emit_ctx *ctx, const compile_file *file, String value);

static void emit_node(emit_ctx *ctx, const compile_file *file, InterpreterFileNodeRef it)
{
    const Node *node = Interpreter_lookup_file_node(ctx->interpreter, it);
    switch (node->kind.val) {
        case Node_INVALID:
        case Node_ListBegin:
        case Node_ListEnd:
        case Node_Ref:
            unreachable();
        case Node_Atom:
            emit_atom(ctx, file, node->u.Atom.value);
            break;
        case Node_Integral:
            emit_integral(ctx, file, node->u.Integral.value);
            break;
        case Node_String:
            emit_string(ctx, file, node->u.String.value);
            break;
    }
}

static void emit_value(emit_ctx *ctx, const compile_file *file, const Value *it)
{
    if (it->type.value == ctx->interpreter->types->t_int.value) {
        emit_integral(ctx, file, it->u.integral.value);
        return;
    }
    if (it->type.value == ctx->interpreter->types->t_string.value) {
        emit_string(ctx, file, it->u.string.value);
        return;
    }
    unreachable();
}

static void emit_atom(emit_ctx *ctx, const compile_file *file, String value)
{
    Target_identifier(ctx->target, ctx->interpreter, file, value);
}

static void emit_integral(emit_ctx *ctx, const compile_file *file, size_t value)
{
    (void) ctx;
    fprintf_zu(file->out, value);
}

static void emit_string(emit_ctx *ctx, const compile_file *file, String value)
{
    (void) ctx;
    fprintf_s(file->out, STR("\""));
    const StringEncoding *enc = value.encoding;
    const uint8_t *begin = String_begin(value);
    Slice(uint8_t) it = value.bytes;
    for (; Slice_begin(&it) != Slice_end(&it); ) {
        size_t c = enc->get(it);
        String replace;
        switch (c) {
            default:
                it = enc->next(it);
                continue;
#define X(c, s) case c: replace = STR(s); break;
            X('\n', "\\n")
#undef X
        }
        fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
        fprintf_s(file->out, replace);
        it = enc->next(it);
        begin = Slice_begin(&it);
    }
    fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin.r = begin, ._end = Slice_begin(&it)}, enc));
    fprintf_s(file->out, STR("\""));
}

static void emit_ref(emit_ctx *ctx, const compile_file *file, InterpreterFileNodeRef ref)
{
    (void) ctx;
    fprintf_s(file->out, STR("_"));
    fprintf_zu(file->out, ref.node.id);
}

static void emit_return_ref(emit_ctx *ctx, const compile_file *file, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
            emit_ref(ctx, file, ret.u.temporary.val);
            return;
        case RETURN_NAMED:
            emit_atom(ctx, file, ret.u.named.val);
            return;
    }
    unreachable();
}

static void emit_return_declare(emit_ctx *ctx, const compile_file *file, TypeRef T, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            Target_var_begin(ctx->target, ctx->interpreter, file, T);
            emit_return_ref(ctx, file, ret);
            Target_var_end(ctx->target, ctx->interpreter, file, T);
            SEMI();
            return;
        }
    }
    unreachable();
}

static void emit_return_assign(emit_ctx *ctx, const compile_file *file, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
            return;
        case RETURN_FUNC:
            fprintf_s(file->out, STR("return "));
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED:
            emit_return_ref(ctx, file, ret);
            fprintf_s(file->out, STR(" = "));
            return;
    }
    unreachable();
}

// visit

static void visit_node_primary(emit_ctx *ctx, const compile_file *file, return_t ret, InterpreterFileNodeRef it);

static void visit_node_list(emit_ctx *ctx, const compile_file *file, return_t ret, InterpreterFileNodeRef it);

static void visit_node(emit_ctx *ctx, const compile_file *file, return_t ret, InterpreterFileNodeRef it)
{
    const Node *node = Interpreter_lookup_file_node(ctx->interpreter, it);
    if (node->kind.val != Node_ListBegin) {
        visit_node_primary(ctx, file, ret, it);
        SEMI();
        return;
    }
    visit_node_list(ctx, file, ret, it);
}

static void visit_node_primary(emit_ctx *ctx, const compile_file *file, return_t ret, InterpreterFileNodeRef it)
{
    const Node *node = Interpreter_lookup_file_node(ctx->interpreter, it);
    switch (node->kind.val) {
        case Node_INVALID:
        case Node_Ref:
        case Node_ListBegin:
        case Node_ListEnd:
            break;
        case Node_Atom:
        case Node_Integral:
        case Node_String:
            emit_return_assign(ctx, file, ret);
            emit_node(ctx, file, it);
            return;
    }
    unreachable();
}

static bool visit_node_macro(emit_ctx *ctx, const compile_file *file, return_t ret,
                             InterpreterFileNodeRef func, Slice(InterpreterFileNodeRef) children);

static void visit_node_expr(emit_ctx *ctx, const compile_file *file, return_t ret,
                            InterpreterFileNodeRef func, Slice(InterpreterFileNodeRef) children);

static void visit_node_list(emit_ctx *ctx, const compile_file *file, return_t ret,
                            InterpreterFileNodeRef it)
{
    Allocator *allocator = ctx->interpreter->allocator;
    assert(Interpreter_lookup_file_node(ctx->interpreter, it)->kind.val == Node_ListBegin && "it is list");
    NodeList childrenRaw = NodeList_iterator(ctx->interpreter, it);
    const size_t n = childrenRaw._n;
    InterpreterFileNodeRef ref = NodeList_get(&childrenRaw, 0);
    InterpreterFileNodeRef first = Interpreter_lookup_node_ref(ctx->interpreter, ref);
    if (n == 1) {
        visit_node(ctx, file, ret, first);
        return;
    }
    Vector(InterpreterFileNodeRef) _children = Vector_new(allocator);
    Vector_push(&_children, first);
    for (size_t i = 1; i < n; ++i) {
        InterpreterFileNodeRef childref = NodeList_get(&childrenRaw, i);
        InterpreterFileNodeRef d = Interpreter_lookup_node_ref(ctx->interpreter, childref);
        Vector_push(&_children, d);
    }
    const Slice(InterpreterFileNodeRef) children = Vector_toSlice(InterpreterFileNodeRef, &_children);
    do {
        if (visit_node_macro(ctx, file, ret, first, children)) {
            break;
        }
        visit_node_expr(ctx, file, ret, first, children);
    } while (false);
    Vector_delete(InterpreterFileNodeRef, &_children);
}

static void visit_node_expr(emit_ctx *ctx, const compile_file *file, return_t ret,
                            InterpreterFileNodeRef func, Slice(InterpreterFileNodeRef) children)
{
    Allocator *allocator = ctx->interpreter->allocator;
    const size_t n = Slice_size(&children);
    (void) n;
    fprintf_s(file->out, STR("{"));
    LINE(+1);

    Value f = eval_node(ctx->interpreter, func);
    const Type *type = Types_lookup(ctx->interpreter->types, f.type);
    assert(type->kind.val == Type_Function);

    const return_t outFunc = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = func};
    emit_return_declare(ctx, file, f.type, outFunc);
    LINE();
    visit_node(ctx, file, outFunc, func);
    LINE();

    const Type *argp = type;
    size_t i = 0;
    while (true) {
        i += 1;
        const TypeRef arg = argp->u.Function.in;
        if (arg.value != ctx->interpreter->types->t_unit.value) {
            InterpreterFileNodeRef ref = *Slice_at(&children, i);
            const return_t outArg = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = ref};
            emit_return_declare(ctx, file, arg, outArg);
            LINE();
            visit_node(ctx, file, outArg, ref);
            LINE();
        }
        const Type *next = Types_lookup(ctx->interpreter->types, argp->u.Function.out);
        if (next->kind.val != Type_Function) {
            break;
        }
        argp = next;
    }
    assert(i == (n - 1) && "consumed all arguments");

    emit_return_assign(ctx, file, ret);
    emit_ref(ctx, file, func);
    fprintf_s(file->out, STR("("));

    argp = type;
    i = 0;
    bool first = true;
    while (true) {
        i += 1;
        const TypeRef arg = argp->u.Function.in;
        if (arg.value != ctx->interpreter->types->t_unit.value) {
            if (!first) {
                fprintf_s(file->out, STR(", "));
            }
            emit_ref(ctx, file, *Slice_at(&children, i));
            first = false;
        }
        const Type *next = Types_lookup(ctx->interpreter->types, argp->u.Function.out);
        if (next->kind.val != Type_Function) {
            break;
        }
        argp = next;
    }
    assert(i == (n - 1) && "consumed all arguments");
    fprintf_s(file->out, STR(")"));
    SEMI();

    LINE(-1);
    fprintf_s(file->out, STR("}"));
}

static bool visit_node_macro(emit_ctx *ctx, const compile_file *file, return_t ret,
                             InterpreterFileNodeRef func, Slice(InterpreterFileNodeRef) children)
{
    Allocator *allocator = ctx->interpreter->allocator;
    const Node *funcNode = Interpreter_lookup_file_node(ctx->interpreter, func);
    if (funcNode->kind.val != Node_Atom) {
        return false;
    }
    Symbol entry;
    bool found = Symbols_lookup(ctx->interpreter->symbols, funcNode->u.Atom.value, &entry);
    if (!found) {
        return false;
    }
    if (!entry.value.flags.intrinsic) {
        return false;
    }
    struct Intrinsic *intrin = entry.value.u.intrinsic.value;

    if (intrin == &intrin_define) {
        const Node *name = Interpreter_lookup_file_node(ctx->interpreter, *Slice_at(&children, 1));
        assert(name->kind.val == Node_Atom);
        InterpreterFileNodeRef ref = *Slice_at(&children, 2);
        Value v = eval_node(ctx->interpreter, ref);
        assert(v.type.value != ctx->interpreter->types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.Atom.value,
        };
        emit_return_declare(ctx, file, v.type, out);
        LINE();
        visit_node(ctx, file, out, ref);
        return true;
    }
    if (intrin == &intrin_set) {
        const Node *name = Interpreter_lookup_file_node(ctx->interpreter, *Slice_at(&children, 1));
        assert(name->kind.val == Node_Atom);
        InterpreterFileNodeRef ref = *Slice_at(&children, 2);
        Value v = eval_node(ctx->interpreter, ref);
        (void) v;
        assert(v.type.value != ctx->interpreter->types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.Atom.value,
        };
        visit_node(ctx, file, out, ref);
        return true;
    }
    if (intrin == &intrin_do) {
        InterpreterFileNodeRef bodyRef = *Slice_at(&children, 1);
        Symbols_push(ctx->interpreter->symbols);
        NodeList iter = NodeList_iterator(ctx->interpreter, bodyRef);
        InterpreterFileNodeRef ref;
        for (size_t i = 0; NodeList_next(&iter, &ref); ++i) {
            if (i) {
                LINE();
            }
            const bool last = i == iter._n - 1;
            ref = Interpreter_lookup_node_ref(ctx->interpreter, ref);
            const return_t out = last ? ret : (return_t) {
                    .kind = RETURN_TEMPORARY,
                    .u.temporary.val = ref,
            };
            Value v = eval_node(ctx->interpreter, ref);
            if (!last && v.type.value != ctx->interpreter->types->t_unit.value) {
                emit_return_declare(ctx, file, v.type, out);
                LINE();
            }
            visit_node(ctx, file, out, ref);
        }
        Symbols_pop(ctx->interpreter->symbols);
        return true;
    }
    if (intrin == &intrin_if) {
        InterpreterFileNodeRef predRef = *Slice_at(&children, 1);
        InterpreterFileNodeRef bodyRef = *Slice_at(&children, 2);
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        emit_return_declare(ctx, file, ctx->interpreter->types->t_int, out);
        LINE();
        visit_node(ctx, file, out, predRef);
        LINE();
        fprintf_s(file->out, STR("if ("));
        emit_ref(ctx, file, predRef);
        fprintf_s(file->out, STR(") {"));
        LINE(+1);
        {
            // todo: same as #do
            visit_node(ctx, file, (return_t) {
                    .kind = RETURN_NO,
            }, bodyRef);
        }
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        return true;
    }
    if (intrin == &intrin_untyped) {
        const Node *code = Interpreter_lookup_file_node(ctx->interpreter, *Slice_at(&children, 1));
        assert(code->kind.val == Node_String);
        emit_return_assign(ctx, file, ret);
        fprintf_s(file->out, code->u.String.value);
        SEMI();
        return true;
    }
    if (intrin == &intrin_while) {
        InterpreterFileNodeRef predRef = *Slice_at(&children, 1);
        InterpreterFileNodeRef bodyRef = *Slice_at(&children, 2);
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        fprintf_s(file->out, STR("for (;;) {"));
        LINE(+1);
        {
            emit_return_declare(ctx, file, ctx->interpreter->types->t_int, out);
            LINE();

            visit_node(ctx, file, out, predRef);
            LINE();

            fprintf_s(file->out, STR("if (!"));
            emit_ref(ctx, file, predRef);
            fprintf_s(file->out, STR(") break"));
            SEMI();
            LINE();

            // todo: same as #do
            visit_node(ctx, file, (return_t) {
                    .kind = RETURN_NO,
            }, bodyRef);
        }
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        return true;
    }
    return false;
}
