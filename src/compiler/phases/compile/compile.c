#include <system.h>
#include "compile.h"

#include <lib/stdio.h>

#include "../../intrinsics/func.h"

typedef struct {
    const ctx_t *ctx;
    FILE *out;
} compile_ctx_t;

static void print_function(const compile_ctx_t *ctx, type_id id, String ident, const String idents[]);

static void print_declaration(const compile_ctx_t *ctx, type_id id, String ident);

typedef enum {
    RETURN_NO,
    RETURN_FUNC,
    RETURN_TEMPORARY,
    RETURN_NAMED,
} return_e;

typedef struct {
    return_e kind;
    uint8_t padding[4];
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

void do_compile(const ctx_t *g_ctx, FILE *out) {
    const compile_ctx_t ctx_ = {.ctx = g_ctx, .out = out};
    const compile_ctx_t *ctx = &ctx_;

    sym_t entry;
    sym_lookup(ctx->ctx, STR("main"), &entry);
    (void) entry;
    assert(type_lookup(ctx->ctx, entry.value.type)->kind == TYPE_FUNCTION);

    const sym_scope_t *globals = &Vector_data(&ctx->ctx->state.symbols.scopes)[0];
    Slice_loop(&Vector_toSlice(TrieEntry, &globals->t.entries), i) {
        const TrieEntry *e = &Vector_data(&globals->t.entries)[i];
        const TrieNode(sym_t) *nod = &Vector_data(&globals->t.nodes)[e->value];
        const sym_t *sym = &nod->value;
        if (sym->flags.intrinsic) {
            continue;
        }
        const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
        const type_id type = sym->type;
        if (sym->flags.native) {
            fprintf_s(ctx->out, STR("extern "));
        }
        if (type_lookup(ctx->ctx, type)->kind == TYPE_FUNCTION) {
            print_function(ctx, type, ident, NULL);
        } else {
            print_declaration(ctx, type, ident);
            if (!sym->flags.native) {
                fprintf_s(ctx->out, STR(" = "));
                // todo: sym->value
            }
        }
        fprintf_s(ctx->out, STR(";\n"));
    }
    Slice_loop(&Vector_toSlice(TrieEntry, &globals->t.entries), i) {
        const TrieEntry *e = &Vector_data(&globals->t.entries)[i];
        const TrieNode(sym_t) *nod = &Vector_data(&globals->t.nodes)[e->value];
        const sym_t *sym = &nod->value;
        if (sym->flags.intrinsic || sym->flags.native) {
            continue;
        }
        const String ident = String_fromSlice(e->key, ENCODING_DEFAULT);
        const type_id type = sym->type;
        if (type_lookup(ctx->ctx, type)->kind == TYPE_FUNCTION) {
            fprintf_s(ctx->out, STR("\n"));
            const node_id args = nod->value.value.u.func.arglist;
            const node_t *argv = node_get(ctx->ctx, args);
            const size_t argc = argv->u.list.size;
            String argnames[argc];
            func_args_names(ctx->ctx, node_list_children(argv), argc, argnames);
            print_function(ctx, type, ident, argnames);
            fprintf_s(ctx->out, STR("\n{\n"));

            const node_id impl = nod->value.value.u.func.value;
            visit_node(ctx, (visit_state_t) {.depth = 1}, (return_t) {.kind = RETURN_FUNC}, node_get(ctx->ctx, impl));

            fprintf_s(ctx->out, STR("\n}\n"));
        }
    }
}

static String type_name(const compile_ctx_t *ctx, type_id id) {
#define CASE(T) if (id.value == ctx->ctx->state.types.T.value)
    CASE(t_unit) return STR("void");
    CASE(t_int) return STR("int");
    CASE(t_string) return STR("const char*");
#undef CASE
    assert(false);
    return STR("?");
}

static void print_function_ret(const compile_ctx_t *ctx, type_id id);

static void print_function_args(const compile_ctx_t *ctx, type_id id, const String idents[]);

static void print_function(const compile_ctx_t *ctx, type_id id, String ident, const String idents[]) {
    print_function_ret(ctx, id);
    fprintf_s(ctx->out, ident);
    print_function_args(ctx, id, idents);
}

static void print_declaration(const compile_ctx_t *ctx, type_id id, String ident) {
    const type_t *T = type_lookup(ctx->ctx, id);
    if (T->kind == TYPE_FUNCTION) {
        print_function_ret(ctx, id);
        fprintf_s(ctx->out, STR("(*"));
        if (String_sizeBytes(ident)) {
            fprintf_s(ctx->out, ident);
        }
        fprintf_s(ctx->out, STR(")"));
        print_function_args(ctx, id, NULL);
        return;
    }
    fprintf_s(ctx->out, type_name(ctx, id));
    if (String_sizeBytes(ident)) {
        fprintf_s(ctx->out, STR(" "));
        fprintf_s(ctx->out, ident);
    }
}

static void print_function_ret(const compile_ctx_t *ctx, type_id id) {
    const type_t *T = type_lookup(ctx->ctx, id);
    const type_id ret = type_func_ret(ctx->ctx, T);
    fprintf_s(ctx->out, type_name(ctx, ret));
    fprintf_s(ctx->out, STR(" "));
}

static void print_function_args(const compile_ctx_t *ctx, type_id id, const String idents[]) {
    fprintf_s(ctx->out, STR("("));
    const type_t *T = type_lookup(ctx->ctx, id);
    const type_t *argp = T;
    size_t i = 0;
    while (true) {
        const type_id arg = argp->u.func.in;
        const String s = idents ? idents[i++] : STR("");
        print_declaration(ctx, arg, s);
        const type_t *next = type_lookup(ctx->ctx, argp->u.func.out);
        if (next->kind != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        fprintf_s(ctx->out, STR(", "));
    }
    fprintf_s(ctx->out, STR(")"));
}

#define TAB() fprintf_s(ctx->out, String_indent(state.depth * 4))

static void return_ref(const compile_ctx_t *ctx, visit_state_t state, return_t ret) {
    (void) state;
    switch (ret.kind) {
        case RETURN_TEMPORARY:
            fprintf_s(ctx->out, STR("_"));
            fprintf_zu(ctx->out, ret.u.temporary.val.val);
            return;
        case RETURN_NAMED:
            assert(false); // todo
            return;
        case RETURN_NO:
        case RETURN_FUNC:
            break;
    }
    assert(false);
}

static void return_declare(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it) {
    (void) it;
    switch (ret.kind) {
        case RETURN_TEMPORARY:
        case RETURN_NAMED:
            fprintf_s(ctx->out, STR("auto "));
            return_ref(ctx, state, ret);
            fprintf_s(ctx->out, STR(";"));
            return;
        case RETURN_NO:
        case RETURN_FUNC:
            break;
    }
    assert(false);
}

static void return_assign(const compile_ctx_t *ctx, visit_state_t state, return_t ret) {
    (void) state;
    switch (ret.kind) {
        case RETURN_FUNC:
            fprintf_s(ctx->out, STR("return "));
            return;
        case RETURN_TEMPORARY:
        case RETURN_NAMED:
            return_ref(ctx, state, ret);
            fprintf_s(ctx->out, STR(" = "));
            return;
        case RETURN_NO:
            break;
    }
    assert(false);
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it);

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it) {
    if (visit_node_primary(ctx, state, ret, it)) {
        fprintf_s(ctx->out, STR(";"));
        return;
    }
    visit_node_list(ctx, state, ret, it);
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it) {
    switch (it->kind) {
        case NODE_ATOM:
            return_assign(ctx, state, ret);
            fprintf_s(ctx->out, it->u.atom.value);
            return true;
        case NODE_INTEGRAL:
            return_assign(ctx, state, ret);
            fprintf_zu(ctx->out, it->u.integral.value);
            return true;
        case NODE_STRING:
            return_assign(ctx, state, ret);
            fprintf_s(ctx->out, STR("\""));
            // todo: escape
            fprintf_s(ctx->out, it->u.string.value);
            fprintf_s(ctx->out, STR("\""));
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
                             const node_t *func, size_t _n, const node_t *children[_n]);

static void visit_node_expr(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *func, size_t n, const node_t *children[n]);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret, const node_t *it) {
    assert(it->kind == NODE_LIST_BEGIN);
    const node_t *childrenRaw = node_list_children(it);
    const size_t n = it->u.list.size;
    const node_t *children[n];
    for (size_t i = 0; i < n; ++i) {
        children[i] = node_deref(ctx->ctx, &childrenRaw[i]);
    }
    const node_t *first = children[0];
    if (n == 1) {
        visit_node(ctx, state, ret, first);
        return;
    }
    if (visit_node_macro(ctx, state, ret, first, n, children)) {
        return;
    }
    visit_node_expr(ctx, state, ret, first, n, children);
}

static void visit_node_expr(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *func, size_t n, const node_t *children[n]) {
    fprintf_s(ctx->out, STR("{\n"));
    state.depth++;

    for (size_t i = 0; i < n; ++i) {
        const node_t *it = children[i];
        const return_t out = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = node_ref(ctx->ctx, it)};
        TAB();
        return_declare(ctx, state, out, it);
        fprintf_s(ctx->out, STR("\n"));
        TAB();
        visit_node(ctx, state, out, it);
        fprintf_s(ctx->out, STR("\n"));
    }
    TAB();
    return_assign(ctx, state, ret);
    fprintf_s(ctx->out, STR("_"));
    fprintf_zu(ctx->out, node_ref(ctx->ctx, func).val);
    fprintf_s(ctx->out, STR("("));
    for (size_t i = 1; i < n; ++i) {
        const bool last = i == n - 1;
        fprintf_s(ctx->out, STR("_"));
        fprintf_zu(ctx->out, node_ref(ctx->ctx, children[i]).val);
        if (!last) {
            fprintf_s(ctx->out, STR(", "));
        }
    }
    fprintf_s(ctx->out, STR(");\n"));

    state.depth--;
    TAB();
    fprintf_s(ctx->out, STR("}"));
}

// fixme: check the value, not the name
static bool visit_node_macro(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                             const node_t *func, size_t _n, const node_t *children[_n]) {
    if (func->kind != NODE_ATOM) {
        return false;
    }
    if (String_equals(func->u.atom.value, STR("#do"))) {
        // todo: extract
        const node_t *bodyNode = children[1];
        assert(bodyNode->kind == NODE_LIST_BEGIN);
        const size_t n = bodyNode->u.list.size;
        const node_t *bodyChildren = node_list_children(bodyNode);
        for (size_t i = 0; i < n; ++i) {
            const bool last = i == n - 1;
            const node_t *it = node_deref(ctx->ctx, &bodyChildren[i]);
            const return_t out = last ? ret : (return_t) {
                    .kind = RETURN_TEMPORARY,
                    .u.temporary.val = node_ref(ctx->ctx, it)
            };
            if (!last) {
                TAB();
                return_declare(ctx, state, out, it);
                fprintf_s(ctx->out, STR("\n"));
            }
            TAB();
            visit_node(ctx, state, out, it);
            if (!last) {
                fprintf_s(ctx->out, STR("\n"));
            }
        }
    } else if (String_equals(func->u.atom.value, STR("#if"))) {
        const node_t *predNode = children[1];
        const node_t *bodyNode = children[2];
        const return_t out = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = node_ref(ctx->ctx, predNode)};
        // don't need first TAB()
        return_declare(ctx, state, out, predNode);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        visit_node(ctx, state, out, predNode);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        fprintf_s(ctx->out, STR("if ("));
        fprintf_s(ctx->out, STR("_"));
        fprintf_zu(ctx->out, node_ref(ctx->ctx, predNode).val);
        fprintf_s(ctx->out, STR(") {\n"));
        {
            state.depth++;
            // todo: same as #do
            visit_node(ctx, state, ret, bodyNode);
            fprintf_s(ctx->out, STR("\n"));
            state.depth--;
        }
        TAB();
        fprintf_s(ctx->out, STR("}"));
    } else if (String_equals(func->u.atom.value, STR("#while"))) {
        const node_t *predNode = children[1];
        const node_t *bodyNode = children[2];
        const return_t out = (return_t) {.kind = RETURN_TEMPORARY, .u.temporary.val = node_ref(ctx->ctx, predNode)};
        // don't need first TAB()
        return_declare(ctx, state, out, predNode);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        visit_node(ctx, state, out, predNode);
        fprintf_s(ctx->out, STR("\n"));

        TAB();
        fprintf_s(ctx->out, STR("while ("));
        fprintf_s(ctx->out, STR("_"));
        fprintf_zu(ctx->out, node_ref(ctx->ctx, predNode).val);
        fprintf_s(ctx->out, STR(") {\n"));
        {
            state.depth++;
            // todo: same as #do
            visit_node(ctx, state, ret, bodyNode);
            fprintf_s(ctx->out, STR("\n"));

            TAB();
            visit_node(ctx, state, out, predNode);
            fprintf_s(ctx->out, STR("\n"));

            state.depth--;
        }
        TAB();
        fprintf_s(ctx->out, STR("}"));
    } else {
        return false;
    }
    return true;
}
