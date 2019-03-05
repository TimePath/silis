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

#define LINE(...) MACRO_BEGIN \
    state.depth = state.depth __VA_ARGS__; \
    fprintf_s(ctx->out, STR("\n")); \
    fprintf_s(ctx->out, String_indent(state.depth * 4)); \
MACRO_END

#define SEMI() fprintf_s(ctx->out, STR(";"))

static void print_node(const compile_ctx_t *ctx, const node_t *it);

static void print_value(const compile_ctx_t *ctx, const value_t *it);

typedef enum {
    RETURN_NO,
    RETURN_FUNC,
    RETURN_TEMPORARY,
    RETURN_NAMED,
} return_e;

typedef struct {
    return_e kind;
    uint8_t padding[4];
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

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

compile_output do_compile(compile_input in)
{
    compile_ctx_t _ctx = {
            .out = in.out,
            .target = in.target,
            .env = in.env,
    };
    compile_ctx_t *ctx = &_ctx;

    sym_t entry;
    bool hasMain = sym_lookup(ctx->env.symbols, STR("main"),
    &entry);
    (void) entry;
    (void) hasMain;
    assert(hasMain && type_lookup(ctx->env.types, entry.value.type)->kind == TYPE_FUNCTION && "main is a function");

    visit_state_t state = (visit_state_t) {.depth = 0};

    ctx->target->file_begin(ctx);

    fflush(ctx->env.prelude);
    String prelude = String_fromSlice(Buffer_toSlice(ctx->env.preludeBuf), ENCODING_DEFAULT);
    fprintf_s(ctx->out, prelude);

    const sym_scope_t *globals = &Vector_data(&ctx->env.symbols->scopes)[0];

    Slice_loop(&Vector_toSlice(TrieEntry, &globals->t.entries), i) {
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
            fprintf_s(ctx->out, STR("extern "));
        }
        if (type_lookup(ctx->env.types, type)->kind == TYPE_FUNCTION) {
            ctx->target->func_forward(ctx, type, ident);
        } else {
            ctx->target->var_begin(ctx, type);
            fprintf_s(ctx->out, ident);
            ctx->target->var_end(ctx, type);
            if (!it->value.flags.native) {
                fprintf_s(ctx->out, STR(" = "));
                print_value(ctx, &it->value);
            }
            SEMI();
        }
        LINE();
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
        LINE();
        Slice(node_t) argv = node_list_children(compilation_node(ctx->env.compilation, it->value.u.func.arglist));
        size_t argc = Slice_size(&argv);
        String *argnames = realloc(NULL, sizeof(String) * argc);
        func_args_names(ctx->env, argv, argnames);
        ctx->target->func_declare(ctx, type, ident, argnames);
        free(argnames);
        LINE();
        fprintf_s(ctx->out, STR("{"));
        LINE(+1);
        const node_t *body = compilation_node(ctx->env.compilation, it->value.u.func.value);
        visit_node(ctx, state, (return_t) {.kind = RETURN_FUNC}, body);
        LINE(-1);
        fprintf_s(ctx->out, STR("}"));
        LINE();
    }

    ctx->target->file_end(ctx);
}

// print

static void print_atom(const compile_ctx_t *ctx, String value);

static void print_integral(const compile_ctx_t *ctx, size_t value);

static void print_string(const compile_ctx_t *ctx, String value);

static void print_node(const compile_ctx_t *ctx, const node_t *it)
{
    switch (it->kind) {
        case NODE_INVALID:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
        case NODE_REF:
            assert(false);
        case NODE_ATOM:
            print_atom(ctx, it->u.atom.value);
            break;
        case NODE_INTEGRAL:
            print_integral(ctx, it->u.integral.value);
            break;
        case NODE_STRING:
            print_string(ctx, it->u.string.value);
            break;
    }
}

static void print_value(const compile_ctx_t *ctx, const value_t *it)
{
    if (it->type.value == ctx->env.types->t_int.value) {
        print_integral(ctx, it->u.integral.value);
        return;
    }
    if (it->type.value == ctx->env.types->t_string.value) {
        print_string(ctx, it->u.string.value);
        return;
    }
    assert(false);
}

static void print_atom(const compile_ctx_t *ctx, String value)
{
    ctx->target->identifier(ctx, value);
}

static void print_integral(const compile_ctx_t *ctx, size_t value)
{
    fprintf_zu(ctx->out, value);
}

static void print_string(const compile_ctx_t *ctx, String value)
{
    fprintf_s(ctx->out, STR("\""));
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
        fprintf_s(ctx->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
        fprintf_s(ctx->out, replace);
        it = enc->next(it);
        begin = it._begin;
    }
    fprintf_s(ctx->out, String_fromSlice((Slice(uint8_t)) {._begin = begin, ._end = it._begin}, enc));
    fprintf_s(ctx->out, STR("\""));
}

static void print_ref(const compile_ctx_t *ctx, compilation_node_ref ref)
{
    fprintf_s(ctx->out, STR("_"));
    fprintf_zu(ctx->out, ref.node.id);
}

static void print_return_ref(const compile_ctx_t *ctx, visit_state_t state, return_t ret)
{
    (void) state;
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
            print_ref(ctx, ret.u.temporary.val);
            return;
        case RETURN_NAMED:
            print_atom(ctx, ret.u.named.val);
            return;
    }
    assert(false);
}

static void print_return_declare(const compile_ctx_t *ctx, visit_state_t state, type_id T, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            ctx->target->var_begin(ctx, T);
            print_return_ref(ctx, state, ret);
            ctx->target->var_end(ctx, T);
            SEMI();
            return;
        }
    }
    assert(false);
}

static void print_return_assign(const compile_ctx_t *ctx, visit_state_t state, return_t ret)
{
    (void) state;
    switch (ret.kind) {
        case RETURN_NO:
            return;
        case RETURN_FUNC:
            fprintf_s(ctx->out, STR("return "));
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED:
            print_return_ref(ctx, state, ret);
            fprintf_s(ctx->out, STR(" = "));
            return;
    }
    assert(false);
}

// visit

static void visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it)
{
    if (it->kind != NODE_LIST_BEGIN) {
        visit_node_primary(ctx, state, ret, it);
        SEMI();
        return;
    }
    visit_node_list(ctx, state, ret, it);
}

static void visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it)
{
    switch (it->kind) {
        case NODE_INVALID:
        case NODE_REF:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
            break;
        case NODE_ATOM:
        case NODE_INTEGRAL:
        case NODE_STRING:
            print_return_assign(ctx, state, ret);
            print_node(ctx, it);
            return;
    }
    assert(false);
}

static bool visit_node_macro(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                             const node_t *func, Slice(node_t_ptr) children);

static void visit_node_expr(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *func, Slice(node_t_ptr) children);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it)
{
    assert(it->kind == NODE_LIST_BEGIN && "it is list");
    const Slice(node_t) childrenRaw = node_list_children(it);
    const size_t n = Slice_size(&childrenRaw);
    compilation_node_ref ref = compilation_node_find(ctx->env.compilation, &Slice_begin(&childrenRaw)[0]);
    const node_t *first = compilation_node(ctx->env.compilation, node_deref(ctx->env.compilation, ref));
    if (n == 1) {
        visit_node(ctx, state, ret, first);
        return;
    }
    const node_t **_children = realloc(NULL, sizeof(node_t *) * n);
    _children[0] = first;
    for (size_t i = 1; i < n; ++i) {
        compilation_node_ref ref = compilation_node_find(ctx->env.compilation, &Slice_begin(&childrenRaw)[i]);
        _children[i] = compilation_node(ctx->env.compilation, node_deref(ctx->env.compilation, ref));
    }
    const Slice(node_t_ptr) children = (Slice(node_t_ptr)) {._begin = &_children[0], ._end = &_children[n]};
    do {
        if (visit_node_macro(ctx, state, ret, first, children)) {
            break;
        }
        visit_node_expr(ctx, state, ret, first, children);
    } while (false);
    free(_children);
}

static void visit_node_expr(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *func, Slice(node_t_ptr) children)
{
    compilation_node_ref funcRef = compilation_node_find(ctx->env.compilation, func);

    const size_t n = Slice_size(&children);
    (void) n;
    fprintf_s(ctx->out, STR("{"));
    LINE(+1);

    value_t f = eval_node(ctx->env, func);
    const type_t *type = type_lookup(ctx->env.types, f.type);
    assert(type->kind == TYPE_FUNCTION);

    const return_t outFunc = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = funcRef};
    print_return_declare(ctx, state, f.type, outFunc);
    LINE();
    visit_node(ctx, state, outFunc, func);
    LINE();

    const type_t *argp = type;
    size_t i = 0;
    while (true) {
        i += 1;
        const type_id arg = argp->u.func.in;
        if (arg.value != ctx->env.types->t_unit.value) {
            const node_t *it = Slice_data(&children)[i];
            compilation_node_ref ref = compilation_node_find(ctx->env.compilation, it);
            const return_t outArg = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = ref};
            print_return_declare(ctx, state, arg, outArg);
            LINE();
            visit_node(ctx, state, outArg, it);
            LINE();
        }
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
    }
    assert(i == (n - 1) && "consumed all arguments");

    print_return_assign(ctx, state, ret);
    print_ref(ctx, funcRef);
    fprintf_s(ctx->out, STR("("));

    argp = type;
    i = 0;
    bool first = true;
    while (true) {
        i += 1;
        const type_id arg = argp->u.func.in;
        if (arg.value != ctx->env.types->t_unit.value) {
            if (!first) {
                fprintf_s(ctx->out, STR(", "));
            }
            print_ref(ctx, compilation_node_find(ctx->env.compilation, Slice_data(&children)[i]));
            first = false;
        }
        const type_t *next = type_lookup(ctx->env.types, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
    }
    assert(i == (n - 1) && "consumed all arguments");
    fprintf_s(ctx->out, STR(")"));
    SEMI();

    LINE(-1);
    fprintf_s(ctx->out, STR("}"));
}

static bool visit_node_macro(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                             const node_t *func, Slice(node_t_ptr) children)
{
    if (func->kind != NODE_ATOM) {
        return false;
    }
    sym_t entry;
    bool found = sym_lookup(ctx->env.symbols, func->u.atom.value, &entry);
    if (!found) {
        return false;
    }
    if (!entry.value.flags.intrinsic) {
        return false;
    }
    struct Intrinsic_s *intrin = entry.value.u.intrinsic.value;

    if (intrin == &intrin_define) {
        const node_t *name = Slice_data(&children)[1];
        assert(name->kind == NODE_ATOM);
        const node_t *it = Slice_data(&children)[2];
        value_t v = eval_node(ctx->env, it);
        assert(v.type.value != ctx->env.types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.atom.value,
        };
        print_return_declare(ctx, state, v.type, out);
        LINE();
        visit_node(ctx, state, out, it);
        return true;
    }
    if (intrin == &intrin_set) {
        const node_t *name = Slice_data(&children)[1];
        assert(name->kind == NODE_ATOM);
        const node_t *it = Slice_data(&children)[2];
        value_t v = eval_node(ctx->env, it);
        (void) v;
        assert(v.type.value != ctx->env.types->t_unit.value && "definition is not void");
        const return_t out = (return_t) {
                .kind = RETURN_NAMED,
                .u.named.val = name->u.atom.value,
        };
        visit_node(ctx, state, out, it);
        return true;
    }
    if (intrin == &intrin_do) {
        const node_t *bodyNode = Slice_data(&children)[1];
        const Slice(node_t) bodyChildren = node_list_children(bodyNode);
        sym_push(ctx->env.symbols);
        nodelist iter = nodelist_iterator(bodyChildren, ctx->env.compilation);
        compilation_node_ref ref;
        for (size_t i = 0; nodelist_next(&iter, &ref); ++i) {
            if (i) {
                LINE();
            }
            const bool last = i == iter._n - 1;
            ref = node_deref(ctx->env.compilation, ref);
            const node_t *it = compilation_node(ctx->env.compilation, ref);
            const return_t out = last ? ret : (return_t) {
                    .kind = RETURN_TEMPORARY,
                    .u.temporary.val = ref,
            };
            value_t v = eval_node(ctx->env, it);
            if (!last && v.type.value != ctx->env.types->t_unit.value) {
                print_return_declare(ctx, state, v.type, out);
                LINE();
            }
            visit_node(ctx, state, out, it);
        }
        sym_pop(ctx->env.symbols);
        return true;
    }
    if (intrin == &intrin_if) {
        const node_t *predNode = Slice_data(&children)[1];
        compilation_node_ref predRef = compilation_node_find(ctx->env.compilation, predNode);
        const node_t *bodyNode = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        print_return_declare(ctx, state, ctx->env.types->t_int, out);
        LINE();
        visit_node(ctx, state, out, predNode);
        LINE();
        fprintf_s(ctx->out, STR("if ("));
        print_ref(ctx, predRef);
        fprintf_s(ctx->out, STR(") {"));
        LINE(+1);
        {
            // todo: same as #do
            visit_node(ctx, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyNode);
        }
        LINE(-1);
        fprintf_s(ctx->out, STR("}"));
        return true;
    }
    if (intrin == &intrin_untyped) {
        const node_t *code = Slice_data(&children)[1];
        assert(code->kind == NODE_STRING);
        fprintf_s(ctx->out, code->u.string.value);
        return true;
    }
    if (intrin == &intrin_while) {
        const node_t *predNode = Slice_data(&children)[1];
        compilation_node_ref predRef = compilation_node_find(ctx->env.compilation, predNode);
        const node_t *bodyNode = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = predRef,
        };
        fprintf_s(ctx->out, STR("for (;;) {"));
        LINE(+1);
        {
            print_return_declare(ctx, state, ctx->env.types->t_int, out);
            LINE();

            visit_node(ctx, state, out, predNode);
            LINE();

            fprintf_s(ctx->out, STR("if (!"));
            print_ref(ctx, predRef);
            fprintf_s(ctx->out, STR(") break"));
            SEMI();
            LINE();

            // todo: same as #do
            visit_node(ctx, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyNode);
        }
        LINE(-1);
        fprintf_s(ctx->out, STR("}"));
        return true;
    }
    return false;
}
