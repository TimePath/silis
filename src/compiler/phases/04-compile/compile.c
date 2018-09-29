#include <system.h>
#include "compile.h"

#include <lib/stdio.h>

#include <compiler/symbols.h>
#include <compiler/value.h>
#include <compiler/intrinsics/do.h>
#include <compiler/intrinsics/func.h>
#include <compiler/intrinsics/if.h>
#include <compiler/intrinsics/while.h>
#include <compiler/phases/03-eval/eval.h>
#include <compiler/targets/_.h>

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
            node_id val;
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

    ctx->target->file_begin(ctx);

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
            fprintf_s(ctx->out, STR(";\n"));
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
        fprintf_s(ctx->out, STR("\n"));
        Slice(node_t) argv = node_list_children(node_get(ctx->env.nodes, it->value.u.func.arglist));
        size_t argc = Slice_size(&argv);
        String *argnames = realloc(NULL, sizeof(String) * argc);
        func_args_names(ctx->env, argv, argnames);
        ctx->target->func_declare(ctx, type, ident, argnames);
        free(argnames);
        fprintf_s(ctx->out, STR("\n{\n"));

        const node_t *body = node_get(ctx->env.nodes, it->value.u.func.value);
        visit_node(ctx, (visit_state_t) {.depth = 1}, (return_t) {.kind = RETURN_FUNC}, body);

        fprintf_s(ctx->out, STR("\n}\n"));
    }

    ctx->target->file_end(ctx);
}

static void print_node(const compile_ctx_t *ctx, const node_t *it)
{
    switch (it->kind) {
        case NODE_INVALID:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
        case NODE_REF:
            assert(false);
        case NODE_ATOM:
            fprintf_s(ctx->out, it->u.atom.value);
            break;
        case NODE_INTEGRAL:
            fprintf_zu(ctx->out, it->u.integral.value);
            break;
        case NODE_STRING:
            fprintf_s(ctx->out, STR("\""));
            // todo: escape
            fprintf_s(ctx->out, it->u.string.value);
            fprintf_s(ctx->out, STR("\""));
            break;
    }
}

static void print_value(const compile_ctx_t *ctx, const value_t *it)
{
    if (it->type.value == ctx->env.types->t_int.value) {
        fprintf_zu(ctx->out, it->u.integral.value);
        return;
    }
    if (it->type.value == ctx->env.types->t_string.value) {
        fprintf_s(ctx->out, STR("\""));
        // todo: escape
        fprintf_s(ctx->out, it->u.string.value);
        fprintf_s(ctx->out, STR("\""));
        return;
    }
    assert(false);
}

static void print_ref(const compile_ctx_t *ctx, node_id val)
{
    fprintf_s(ctx->out, STR("_"));
    fprintf_zu(ctx->out, val.val);
}

#define TAB() fprintf_s(ctx->out, String_indent(state.depth * 4))

static void return_ref(const compile_ctx_t *ctx, visit_state_t state, return_t ret)
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
            assert(false); // todo
            return;
    }
    assert(false);
}

static void return_declare(const compile_ctx_t *ctx, visit_state_t state, type_id T, return_t ret)
{
    switch (ret.kind) {
        case RETURN_NO:
        case RETURN_FUNC:
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            ctx->target->var_begin(ctx, T);
            return_ref(ctx, state, ret);
            ctx->target->var_end(ctx, T);
            fprintf_s(ctx->out, STR(";"));
            return;
        }
    }
    assert(false);
}

static void return_assign(const compile_ctx_t *ctx, visit_state_t state, return_t ret)
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
            return_ref(ctx, state, ret);
            fprintf_s(ctx->out, STR(" = "));
            return;
    }
    assert(false);
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it)
{
    if (visit_node_primary(ctx, state, ret, it)) {
        fprintf_s(ctx->out, STR(";"));
        return;
    }
    visit_node_list(ctx, state, ret, it);
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it)
{
    switch (it->kind) {
        case NODE_ATOM:
        case NODE_INTEGRAL:
        case NODE_STRING:
            return_assign(ctx, state, ret);
            print_node(ctx, it);
            return true;
        case NODE_LIST_BEGIN: {
            const size_t n = it->u.list.size;
            if (!n) {
                return_assign(ctx, state, ret);
                fprintf_s(ctx->out, STR("void"));
                return true;
            }
            return false;
        }
        case NODE_INVALID:
        case NODE_LIST_END:
        case NODE_REF:
            break;
    }
    assert(false);
    return false;
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
    const node_t *first = node_deref(&Slice_data(&childrenRaw)[0], ctx->env.nodes);
    if (n == 1) {
        visit_node(ctx, state, ret, first);
        return;
    }
    const node_t **_children = realloc(NULL, sizeof(node_t *) * n);
    _children[0] = first;
    for (size_t i = 1; i < n; ++i) {
        _children[i] = node_deref(&Slice_data(&childrenRaw)[i], ctx->env.nodes);
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
    const size_t n = Slice_size(&children);
    fprintf_s(ctx->out, STR("{\n"));
    state.depth++;

    for (size_t i = 0; i < n; ++i) {
        const node_t *it = Slice_data(&children)[i];
        value_t v = eval_node(ctx->env, it);
        const return_t out = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = node_ref(it, ctx->env.nodes)};
        TAB();
        return_declare(ctx, state, v.type, out);
        fprintf_s(ctx->out, STR("\n"));
        TAB();
        visit_node(ctx, state, out, it);
        fprintf_s(ctx->out, STR("\n"));
    }
    TAB();
    return_assign(ctx, state, ret);
    print_ref(ctx, node_ref(func, ctx->env.nodes));
    fprintf_s(ctx->out, STR("("));
    for (size_t i = 1; i < n; ++i) {
        const bool last = i == n - 1;
        print_ref(ctx, node_ref(Slice_data(&children)[i], ctx->env.nodes));
        if (!last) {
            fprintf_s(ctx->out, STR(", "));
        }
    }
    fprintf_s(ctx->out, STR(");\n"));

    state.depth--;
    TAB();
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

    if (intrin == &intrin_do) {
        const node_t *bodyNode = Slice_data(&children)[1];
        const Slice(node_t) bodyChildren = node_list_children(bodyNode);
        const size_t n = Slice_size(&bodyChildren);
        sym_push(ctx->env.symbols, 0);
        for (size_t i = 0; i < n; ++i) {
            const bool last = i == n - 1;
            const node_t *it = node_deref(&Slice_data(&bodyChildren)[i], ctx->env.nodes);
            const return_t out = last ? ret : (return_t) {
                    .kind = RETURN_TEMPORARY,
                    .u.temporary.val = node_ref(it, ctx->env.nodes)
            };
            value_t v = eval_node(ctx->env, it);
            if (!last && v.type.value != ctx->env.types->t_unit.value) {
                TAB();
                return_declare(ctx, state, v.type, out);
                fprintf_s(ctx->out, STR("\n"));
            }
            TAB();
            visit_node(ctx, state, out, it);
            if (!last) {
                fprintf_s(ctx->out, STR("\n"));
            }
        }
        sym_pop(ctx->env.symbols);
        return true;
    }
    if (intrin == &intrin_if) {
        const node_t *predNode = Slice_data(&children)[1];
        const node_t *bodyNode = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = node_ref(predNode, ctx->env.nodes),
        };
        // don't need first TAB()
        return_declare(ctx, state, ctx->env.types->t_int, out);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        visit_node(ctx, state, out, predNode);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        fprintf_s(ctx->out, STR("if ("));
        print_ref(ctx, node_ref(predNode, ctx->env.nodes));
        fprintf_s(ctx->out, STR(") {\n"));
        {
            state.depth++;
            // todo: same as #do
            visit_node(ctx, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyNode);
            fprintf_s(ctx->out, STR("\n"));
            state.depth--;
        }
        TAB();
        fprintf_s(ctx->out, STR("}"));
        return true;
    }
    if (intrin == &intrin_while) {
        const node_t *predNode = Slice_data(&children)[1];
        const node_t *bodyNode = Slice_data(&children)[2];
        const return_t out = (return_t) {
                .kind = RETURN_TEMPORARY,
                .u.temporary.val = node_ref(predNode, ctx->env.nodes),
        };
        // don't need first TAB()
        fprintf_s(ctx->out, STR("for (;;) {\n"));
        {
            state.depth++;

            TAB();
            return_declare(ctx, state, ctx->env.types->t_int, out);
            fprintf_s(ctx->out, STR("\n"));

            TAB();
            visit_node(ctx, state, out, predNode);
            fprintf_s(ctx->out, STR("\n"));

            TAB();
            fprintf_s(ctx->out, STR("if (!"));
            print_ref(ctx, node_ref(predNode, ctx->env.nodes));
            fprintf_s(ctx->out, STR(") break;\n"));

            // todo: same as #do
            visit_node(ctx, state, (return_t) {
                    .kind = RETURN_NO,
            }, bodyNode);
            fprintf_s(ctx->out, STR("\n"));

            state.depth--;
        }
        TAB();
        fprintf_s(ctx->out, STR("}"));
        return true;
    }
    return false;
}
