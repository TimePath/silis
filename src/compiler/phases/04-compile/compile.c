#include <system.h>
#include "compile.h"

#include <lib/stdio.h>

#include <compiler/symbols.h>
#include <compiler/value.h>
#include <compiler/intrinsics/define.h>
#include <compiler/intrinsics/do.h>
#include <compiler/intrinsics/func.h>
#include <compiler/intrinsics/if.h>
#include <compiler/intrinsics/set.h>
#include <compiler/intrinsics/untyped.h>
#include <compiler/intrinsics/while.h>
#include <compiler/phases/03-eval/eval.h>
#include <compiler/targets/_.h>
#include <compiler/env.h>
#include <compiler/type.h>
#include <compiler/compilation.h>

#define LINE(...) MACRO_BEGIN \
    state.depth = state.depth __VA_ARGS__; \
    fprintf_s(file->out, STR("\n")); \
    fprintf_s(file->out, String_indent(state.depth * 4)); \
MACRO_END

#define SEMI() fprintf_s(file->out, STR(";"))

static void print_node(const compile_ctx_t *ctx, const compile_file *file, compilation_node_ref it);

static void print_value(const compile_ctx_t *ctx, const compile_file *file, const value_t *it);

typedef enum {
    RETURN_NO,
    RETURN_FUNC,
    RETURN_TEMPORARY,
    RETURN_NAMED,
} return_e;

typedef struct {
    return_e kind;
    uint8_t _padding[4];
    type_id type;
    union {
        struct {
            compilation_node_ref val;
        } temporary;
        struct {
            String val;
        } named;
    } u;
} return_t;

typedef struct {
    size_t depth;
} visit_state_t;

static void visit_node(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret, compilation_node_ref it);

compile_output do_compile(compile_input in)
{
    Vector(compile_file) files = Vector_new();

    compile_ctx_t _ctx = {
            .target = in.target,
            .env = in.env,
    };
    compile_ctx_t *ctx = &_ctx;

    sym_t entry;
    bool hasMain = sym_lookup(ctx->env.symbols, STR("main"), &entry);
    (void) entry;
    (void) hasMain;
    assert(hasMain && type_lookup(ctx->env.types, entry.value.type)->kind == TYPE_FUNCTION && "main is a function");

    fs_flush(ctx->env.prelude);
    String prelude = String_fromSlice(Buffer_toSlice(ctx->env.preludeBuf), ENCODING_DEFAULT);

    Slice_loop(&Vector_toSlice(compilation_file_ptr_t, &in.env.compilation->files), i) {
        Buffer *content = realloc(NULL, sizeof(*content));
        *content = Buffer_new();
        compile_file x = (compile_file) {
                .file = {i + 1},
                .content = content,
                .out = Buffer_asFile(content),
        };
        Vector_push(&files, x);
    }

    Slice_loop(&Vector_toSlice(compile_file, &files), i) {
        const compile_file *file = &Vector_data(&files)[i];
        ctx->target->file_begin(ctx, file);
        fprintf_s(file->out, prelude);
    }

    visit_state_t state = (visit_state_t) {.depth = 0};

    const sym_scope_t *globals = &Vector_data(&ctx->env.symbols->scopes)[0];
    Slice_loop(&Vector_toSlice(compile_file, &files), j) {
        Slice_loop(&Vector_toSlice(TrieEntry, &globals->t.entries), i) {
            const compile_file *file = &Vector_data(&files)[j];

            const TrieEntry *e = &Vector_data(&globals->t.entries)[i];
            const TrieNode(sym_t) *n = &Vector_data(&globals->t.nodes)[e->value];
            const sym_t _it = n->value;
            const sym_t *it = &_it;
            if (it->value.flags.intrinsic) {
                continue;
            }
            const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
            const type_id type = it->type;
            if (it->value.flags.native) {
                fprintf_s(file->out, STR("extern "));
            }
            if (type_lookup(ctx->env.types, type)->kind == TYPE_FUNCTION) {
                ctx->target->func_forward(ctx, file, type, ident);
            } else {
                ctx->target->var_begin(ctx, file, type);
                fprintf_s(file->out, ident);
                ctx->target->var_end(ctx, file, type);
                if (!it->value.flags.native) {
                    fprintf_s(file->out, STR(" = "));
                    print_value(ctx, file, &it->value);
                }
                SEMI();
            }
            LINE();
        }
    }
    Slice_loop(&Vector_toSlice(TrieEntry, &globals->t.entries), i) {
        const TrieEntry *e = &Vector_data(&globals->t.entries)[i];
        const TrieNode(sym_t) *n = &Vector_data(&globals->t.nodes)[e->value];
        const sym_t _it = n->value;
        const sym_t *it = &_it;
        if (it->value.flags.intrinsic || it->value.flags.native) {
            continue;
        }
        const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
        const type_id type = it->type;
        if (type_lookup(ctx->env.types, type)->kind != TYPE_FUNCTION) {
            continue;
        }
        assert(it->file.id && "global is user-defined");
        const compile_file *file = &Vector_data(&files)[it->file.id - 1];
        LINE();
        nodelist argv = nodelist_iterator(ctx->env.compilation, it->value.u.func.arglist);
        size_t argc = argv._n;
        String *argnames = realloc(NULL, sizeof(String) * argc);
        func_args_names(ctx->env, argv, argnames);
        ctx->target->func_declare(ctx, file, type, ident, argnames);
        free(argnames);
        LINE();
        fprintf_s(file->out, STR("{"));
        LINE(+1);
        compilation_node_ref body = it->value.u.func.value;
        visit_node(ctx, file, state, (return_t) {.kind = RETURN_FUNC}, body);
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        LINE();
    }

    Slice_loop(&Vector_toSlice(compile_file, &files), i) {
        const compile_file *file = &Vector_data(&files)[i];
        ctx->target->file_end(ctx, file);
    }

    return (compile_output) {
            .files = files,
    };
}

// print

static void print_atom(const compile_ctx_t *ctx, const compile_file *file, String value);

static void print_integral(const compile_ctx_t *ctx, const compile_file *file, size_t value);

static void print_string(const compile_ctx_t *ctx, const compile_file *file, String value);

static void print_node(const compile_ctx_t *ctx, const compile_file *file, compilation_node_ref it)
{
    const node_t *node = compilation_node(ctx->env.compilation, it);
    switch (node->kind) {
        case NODE_INVALID:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
        case NODE_REF:
            assert(false);
        case NODE_ATOM:
            print_atom(ctx, file, node->u.atom.value);
            break;
        case NODE_INTEGRAL:
            print_integral(ctx, file, node->u.integral.value);
            break;
        case NODE_STRING:
            print_string(ctx, file, node->u.string.value);
            break;
    }
}

static void print_value(const compile_ctx_t *ctx, const compile_file *file, const value_t *it)
{
    if (it->type.value == ctx->env.types->t_int.value) {
        print_integral(ctx, file, it->u.integral.value);
        return;
    }
    if (it->type.value == ctx->env.types->t_string.value) {
        print_string(ctx, file, it->u.string.value);
        return;
    }
    assert(false);
}

static void print_atom(const compile_ctx_t *ctx, const compile_file *file, String value)
{
    ctx->target->identifier(ctx, file, value);
}

static void print_integral(const compile_ctx_t *ctx, const compile_file *file, size_t value)
{
    (void) ctx;
    fprintf_zu(file->out, value);
}

static void print_string(const compile_ctx_t *ctx, const compile_file *file, String value)
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
        fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
        fprintf_s(file->out, replace);
        it = enc->next(it);
        begin = it._begin;
    }
    fprintf_s(file->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
    fprintf_s(file->out, STR("\""));
}

static void print_ref(const compile_ctx_t *ctx, const compile_file *file, compilation_node_ref ref)
{
    (void) ctx;
    fprintf_s(file->out, STR("_"));
    fprintf_zu(file->out, ref.node.id);
}

static void print_return_ref(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret)
{
    (void) state;
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
            print_ref(ctx, file, ret.u.temporary.val);
            return;
        case RETURN_NAMED:
            print_atom(ctx, file, ret.u.named.val);
            return;
    }
    assert(false);
}

static void print_return_declare(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, type_id T, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            ctx->target->var_begin(ctx, file, T);
            print_return_ref(ctx, file, state, ret);
            ctx->target->var_end(ctx, file, T);
            SEMI();
            return;
        }
    }
    assert(false);
}

static void print_return_assign(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret)
{
    (void) state;
    switch (ret.kind) {
        case RETURN_NO:
            return;
        case RETURN_FUNC:
            fprintf_s(file->out, STR("return "));
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED:
            print_return_ref(ctx, file, state, ret);
            fprintf_s(file->out, STR(" = "));
            return;
    }
    assert(false);
}

// visit

static void visit_node_primary(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret, compilation_node_ref it);

static void visit_node_list(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret, compilation_node_ref it);

static void visit_node(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret, compilation_node_ref it)
{
    const node_t *node = compilation_node(ctx->env.compilation, it);
    if (node->kind != NODE_LIST_BEGIN) {
        visit_node_primary(ctx, file, state, ret, it);
        SEMI();
        return;
    }
    visit_node_list(ctx, file, state, ret, it);
}

static void visit_node_primary(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret, compilation_node_ref it)
{
    const node_t *node = compilation_node(ctx->env.compilation, it);
    switch (node->kind) {
        case NODE_INVALID:
        case NODE_REF:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
            break;
        case NODE_ATOM:
        case NODE_INTEGRAL:
        case NODE_STRING:
            print_return_assign(ctx, file, state, ret);
            print_node(ctx, file, it);
            return;
    }
    assert(false);
}

static bool visit_node_macro(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret,
                             compilation_node_ref func, Slice(compilation_node_ref) children);

static void visit_node_expr(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret,
                            compilation_node_ref func, Slice(compilation_node_ref) children);

static void visit_node_list(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret,
                            compilation_node_ref it)
{
    assert(compilation_node(ctx->env.compilation, it)->kind == NODE_LIST_BEGIN && "it is list");
    nodelist childrenRaw = nodelist_iterator(ctx->env.compilation, it);
    const size_t n = childrenRaw._n;
    compilation_node_ref ref = nodelist_get(&childrenRaw, 0);
    compilation_node_ref first = node_deref(ctx->env.compilation, ref);
    if (n == 1) {
        visit_node(ctx, file, state, ret, first);
        return;
    }
    Vector(compilation_node_ref) _children = Vector_new();
    Vector_push(&_children, first);
    for (size_t i = 1; i < n; ++i) {
        compilation_node_ref childref = nodelist_get(&childrenRaw, i);
        compilation_node_ref d = node_deref(ctx->env.compilation, childref);
        Vector_push(&_children, d);
    }
    const Slice(compilation_node_ref) children = Vector_toSlice(compilation_node_ref, &_children);
    do {
        if (visit_node_macro(ctx, file, state, ret, first, children)) {
            break;
        }
        visit_node_expr(ctx, file, state, ret, first, children);
    } while (false);
    Vector_delete(compilation_node_ref, &_children);
}

static void visit_node_expr(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret,
                            compilation_node_ref func, Slice(compilation_node_ref) children)
{
    const size_t n = Slice_size(&children);
    (void) n;
    fprintf_s(file->out, STR("{"));
    LINE(+1);

    value_t f = eval_node(ctx->env, func);
    const type_t *type = type_lookup(ctx->env.types, f.type);
    assert(type->kind == TYPE_FUNCTION);

    const return_t outFunc = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = func};
    print_return_declare(ctx, file, state, f.type, outFunc);
    LINE();
    visit_node(ctx, file, state, outFunc, func);
    LINE();

    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        i += 1;
        const type_id arg = argp->u.func.in;
        if (arg.value != ctx->env.types->t_unit.value) {
            compilation_node_ref ref = Slice_data(&children)[i];
            const return_t outArg = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = ref};
            print_return_declare(ctx, file, state, arg, outArg);
            LINE();
            visit_node(ctx, file, state, outArg, ref);
            LINE();
        }
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
    }
    assert(i == (n - 1) && "consumed all arguments");

    print_return_assign(ctx, file, state, ret);
    print_ref(ctx, file, func);
    fprintf_s(file->out, STR("("));

    argp = type;
    i = 0;
    bool first = true;
    while (true) {
        i += 1;
        const type_id arg = argp->u.func.in;
        if (arg.value != ctx->env.types->t_unit.value) {
            if (!first) {
                fprintf_s(file->out, STR(", "));
            }
            print_ref(ctx, file, Slice_data(&children)[i]);
            first = false;
        }
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
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

static bool visit_node_macro(const compile_ctx_t *ctx, const compile_file *file, visit_state_t state, return_t ret,
                             compilation_node_ref func, Slice(compilation_node_ref) children)
{
    const node_t *funcNode = compilation_node(ctx->env.compilation, func);
    if (funcNode->kind != NODE_ATOM) {
        return false;
    }
    sym_t entry;
    bool found = sym_lookup(ctx->env.symbols, funcNode->u.atom.value, &entry);
    if (!found) {
        return false;
    }
    if (!entry.value.flags.intrinsic) {
        return false;
    }
    struct Intrinsic_s *intrin = entry.value.u.intrinsic.value;

    if (intrin == &intrin_define) {
        const node_t *name = compilation_node(ctx->env.compilation, Slice_data(&children)[1]);
        assert(name->kind == NODE_ATOM);
        compilation_node_ref ref = Slice_data(&children)[2];
        value_t v = eval_node(ctx->env, ref);
        assert(v.type.value != ctx->env.types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.atom.value,
        };
        print_return_declare(ctx, file, state, v.type, out);
        LINE();
        visit_node(ctx, file, state, out, ref);
        return true;
    }
    if (intrin == &intrin_set) {
        const node_t *name = compilation_node(ctx->env.compilation, Slice_data(&children)[1]);
        assert(name->kind == NODE_ATOM);
        compilation_node_ref ref = Slice_data(&children)[2];
        value_t v = eval_node(ctx->env, ref);
        (void) v;
        assert(v.type.value != ctx->env.types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.atom.value,
        };
        visit_node(ctx, file, state, out, ref);
        return true;
    }
    if (intrin == &intrin_do) {
        compilation_node_ref bodyRef = Slice_data(&children)[1];
        sym_push(ctx->env.symbols);
        nodelist iter = nodelist_iterator(ctx->env.compilation, bodyRef);
        compilation_node_ref ref;
        for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
            if (i) {
                LINE();
            }
            const bool last = i == iter._n - 1;
            ref = node_deref(ctx->env.compilation, ref);
            const return_t out = last ? ret : (return_t) {
                    .kind = RETURN_TEMPORARY,
                    .u.temporary.val = ref,
            };
            value_t v = eval_node(ctx->env, ref);
            if (!last && v.type.value != ctx->env.types->t_unit.value) {
                print_return_declare(ctx, file, state, v.type, out);
                LINE();
            }
            visit_node(ctx, file, state, out, ref);
        }
        sym_pop(ctx->env.symbols);
        return true;
    }
    if (intrin == &intrin_if) {
        compilation_node_ref predRef = Slice_data(&children)[1];
        compilation_node_ref bodyRef = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        print_return_declare(ctx, file, state, ctx->env.types->t_int, out);
        LINE();
        visit_node(ctx, file, state, out, predRef);
        LINE();
        fprintf_s(file->out, STR("if ("));
        print_ref(ctx, file, predRef);
        fprintf_s(file->out, STR(") {"));
        LINE(+1);
        {
            // todo: same as #do
            visit_node(ctx, file, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyRef);
        }
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        return true;
    }
    if (intrin == &intrin_untyped) {
        const node_t *code = compilation_node(ctx->env.compilation, Slice_data(&children)[1]);
        assert(code->kind == NODE_STRING);
        print_return_assign(ctx, file, state, ret);
        fprintf_s(file->out, code->u.string.value);
        SEMI();
        return true;
    }
    if (intrin == &intrin_while) {
        compilation_node_ref predRef = Slice_data(&children)[1];
        compilation_node_ref bodyRef = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        fprintf_s(file->out, STR("for (;;) {"));
        LINE(+1);
        {
            print_return_declare(ctx, file, state, ctx->env.types->t_int, out);
            LINE();

            visit_node(ctx, file, state, out, predRef);
            LINE();

            fprintf_s(file->out, STR("if (!"));
            print_ref(ctx, file, predRef);
            fprintf_s(file->out, STR(") break"));
            SEMI();
            LINE();

            // todo: same as #do
            visit_node(ctx, file, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyRef);
        }
        LINE(-1);
        fprintf_s(file->out, STR("}"));
        return true;
    }
    return false;
}
