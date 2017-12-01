#include "compile.h"

#include "../../intrinsics/func.h"

#include <assert.h>
#include <stdarg.h>

typedef struct {
    const ctx_t *ctx;
    FILE *out;
} compile_ctx_t;


__attribute__((format(printf, 2, 3)))
static void OUT(const compile_ctx_t *ctx, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(ctx->out, format, args);
    va_end(args);
}

static void print_function(const compile_ctx_t *ctx, type_id id, string_view_t ident, string_view_t idents[]);

static void print_declaration(const compile_ctx_t *ctx, type_id id, string_view_t ident);

#ifndef NDEBUG
#define DEBUG_COMPILE
#endif

typedef enum {
    RETURN_NO,
    RETURN_FUNC,
    RETURN_TEMPORARY,
    RETURN_NAMED,
} return_e;

typedef struct {
    return_e tag;
    union {
        struct {
            size_t val;
        } temporary;
        struct {
            string_view_t val;
        } named;
    } u;
} return_t;

typedef struct {
    size_t depth;
} visit_state_t;

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                       const node_t *it);

void do_compile(const ctx_t *g_ctx) {
#ifdef DEBUG_COMPILE
    FILE *out = stdout;
#else
    buffer_t buf;
    FILE *out = buf_file(&buf);
#endif
    compile_ctx_t ctx_ = {.ctx = g_ctx, .out = out};
    compile_ctx_t *ctx = &ctx_;

    const sym_t *entry = sym_lookup(ctx->ctx, STR("main"));
    assert(type_lookup(ctx->ctx, entry->value.type).type == TYPE_FUNCTION);

    const sym_trie_t *globals = &ctx->ctx->state.symbols.scopes.data[0];
    vec_loop(globals->list, i, 0) {
        const sym_trie_entry_t *e = &globals->list.data[i];
        const sym_trie_node_t *nod = &globals->nodes.data[e->value];
        const sym_t *sym = &nod->value;
        if (sym->flags.intrinsic) {
            continue;
        }
        const string_view_t ident = e->key;
        const type_id type = sym->type;
        if (sym->flags.native) {
            OUT(ctx, "extern ");
        }
        if (type_lookup(ctx->ctx, type).type == TYPE_FUNCTION) {
            print_function(ctx, type, ident, NULL);
        } else {
            print_declaration(ctx, type, ident);
            if (!sym->flags.native) {
                OUT(ctx, " = ");
                // todo: sym->value
            }
        }
        OUT(ctx, ";\n");
    }
    vec_loop(globals->list, i, 0) {
        const sym_trie_entry_t *e = &globals->list.data[i];
        const sym_trie_node_t *nod = &globals->nodes.data[e->value];
        const sym_t *sym = &nod->value;
        if (sym->flags.intrinsic || sym->flags.native) {
            continue;
        }
        const string_view_t ident = e->key;
        const type_id type = sym->type;
        if (type_lookup(ctx->ctx, type).type == TYPE_FUNCTION) {
            OUT(ctx, "\n");
            const node_ref_t args = nod->value.value.u.func.arglist;
            const node_t *argv = ctx_node(ctx->ctx, args);
            const size_t argc = argv->u.list.size;
            string_view_t argnames[argc];
            func_args_names(ctx->ctx, NODE_LIST_CHILDREN(argv), argc, argnames);
            print_function(ctx, type, ident, argnames);
            OUT(ctx, "\n{\n");

            const node_ref_t impl = nod->value.value.u.func.value;
            visit_node(ctx, (visit_state_t) {.depth = 1}, (return_t) {.tag = RETURN_FUNC}, ctx_node(ctx->ctx, impl));

            OUT(ctx, "\n}\n");
        }
    }
#ifdef DEBUG_COMPILE
#else
    fclose(out);
    fprintf(stdout, BUF_PRINTF, BUF_PRINTF_PASS(buf));
#endif
}

static string_view_t typename(const compile_ctx_t *ctx, type_id id) {
#define CASE(T) if (id.value == ctx->ctx->state.types.T.value)
    CASE(t_unit) return STR("void");
    CASE(t_int) return STR("int");
    CASE(t_string) return STR("const char*");
#undef CASE
    assert(false);
}

static void print_function_ret(const compile_ctx_t *ctx, type_id id);

static void print_function_args(const compile_ctx_t *ctx, type_id id, string_view_t idents[]);

static void print_function(const compile_ctx_t *ctx, type_id id, string_view_t ident, string_view_t idents[]) {
    print_function_ret(ctx, id);
    OUT(ctx, STR_PRINTF, STR_PRINTF_PASS(ident));
    print_function_args(ctx, id, idents);
}

static void print_declaration(const compile_ctx_t *ctx, type_id id, string_view_t ident) {
    const type_t T = type_lookup(ctx->ctx, id);
    if (T.type == TYPE_FUNCTION) {
        print_function_ret(ctx, id);
        OUT(ctx, "(*");
        if (str_size(ident)) {
            OUT(ctx, STR_PRINTF, STR_PRINTF_PASS(ident));
        }
        OUT(ctx, ")");
        print_function_args(ctx, id, NULL);
        return;
    }
    OUT(ctx, STR_PRINTF, STR_PRINTF_PASS(typename(ctx, id)));
    if (str_size(ident)) {
        OUT(ctx, " " STR_PRINTF, STR_PRINTF_PASS(ident));
    }
}

static void print_function_ret(const compile_ctx_t *ctx, type_id id) {
    const type_t T = type_lookup(ctx->ctx, id);
    const type_id ret = type_func_ret(ctx->ctx, T);
    OUT(ctx, STR_PRINTF " ", STR_PRINTF_PASS(typename(ctx, ret)));
}

static void print_function_args(const compile_ctx_t *ctx, type_id id, string_view_t idents[]) {
    OUT(ctx, "(");
    const type_t T = type_lookup(ctx->ctx, id);
    type_t argp = T;
    size_t i = 0;
    while (true) {
        const type_id arg = argp.u.func.in;
        string_view_t s = idents ? idents[i++] : STR("");
        print_declaration(ctx, arg, s);
        const type_t next = type_lookup(ctx->ctx, argp.u.func.out);
        if (next.type != TYPE_FUNCTION) {
            break;
        }
        argp = next;
        OUT(ctx, ", ");
    }
    OUT(ctx, ")");
}

#define TAB() OUT(ctx, STR_PRINTF, STR_PRINTF_PASS(str_indent(state.depth * 4)))

static void return_ref(const compile_ctx_t *ctx, visit_state_t state, return_t ret) {
    (void) state;
    switch (ret.tag) {
        default: {
            assert(false);
        }
        case RETURN_TEMPORARY: {
            OUT(ctx, "_%lu", ret.u.temporary.val);
            break;
        }
        case RETURN_NAMED: {
            assert(false); // todo
            break;
        }
    }
}

static void return_declare(const compile_ctx_t *ctx, visit_state_t state, return_t ret) {
    switch (ret.tag) {
        default: {
            assert(false);
            break;
        }
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            OUT(ctx, "auto ");
            return_ref(ctx, state, ret);
            OUT(ctx, ";");
            break;
        }
    }
}

static void return_assign(const compile_ctx_t *ctx, visit_state_t state, return_t ret) {
    (void) state;
    switch (ret.tag) {
        default: {
            assert(false);
            break;
        }
        case RETURN_FUNC: {
            OUT(ctx, "return ");
            break;
        }
        case RETURN_TEMPORARY:
        case RETURN_NAMED: {
            return_ref(ctx, state, ret);
            OUT(ctx, " = ");
            break;
        }
    }
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                               const node_t *it);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *it);

static void visit_node(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                       const node_t *it) {
    if (visit_node_primary(ctx, state, ret, it)) {
        OUT(ctx, ";");
        return;
    }
    visit_node_list(ctx, state, ret, it);
}

static bool visit_node_primary(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                               const node_t *it) {
    switch (it->type) {
        default:
            assert(false);
            break;
        case NODE_ATOM:
            return_assign(ctx, state, ret);
            OUT(ctx, STR_PRINTF, STR_PRINTF_PASS(it->u.atom.value));
            return true;
        case NODE_INTEGRAL:
            return_assign(ctx, state, ret);
            OUT(ctx, "%lu", it->u.integral.value);
            return true;
        case NODE_STRING:
            return_assign(ctx, state, ret);
            // todo: escape
            OUT(ctx, "\"" STR_PRINTF "\"", STR_PRINTF_PASS(it->u.string.value));
            return true;
        case NODE_LIST_BEGIN: {
            const size_t n = it->u.list.size;
            if (!n) {
                return_assign(ctx, state, ret);
                OUT(ctx, "void");
                return true;
            }
            break;
        }
    }
    return false;
}

static bool visit_node_macro(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                             const node_t *func, size_t _n, const node_t *children[_n]);

static void visit_node_expr(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *func, size_t n, const node_t *children[n]);

static void visit_node_list(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                            const node_t *it) {
    assert(it->type == NODE_LIST_BEGIN);
    const node_t *childrenRaw = NODE_LIST_CHILDREN(it);
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
    OUT(ctx, "{\n");
    state.depth++;

    for (size_t i = 0; i < n; ++i) {
        const node_t *it = children[i];
        const return_t out = (return_t) {.tag = RETURN_TEMPORARY, .u.temporary.val = node_id(ctx->ctx, it)};
        TAB();
        return_declare(ctx, state, out);
        OUT(ctx, "\n");
        TAB();
        visit_node(ctx, state, out, it);
        OUT(ctx, "\n");
    }
    TAB();
    return_assign(ctx, state, ret);
    OUT(ctx, "_%lu(", node_id(ctx->ctx, func));
    for (size_t i = 1; i < n; ++i) {
        const bool last = i == n - 1;
        OUT(ctx, "_%lu", node_id(ctx->ctx, children[i]));
        if (!last) {
            OUT(ctx, ", ");
        }
    }
    OUT(ctx, ");\n");

    state.depth--;
    TAB();
    OUT(ctx, "}");
}

// fixme: check the value, not the name
static bool visit_node_macro(const compile_ctx_t *ctx, visit_state_t state, return_t ret,
                             const node_t *func, size_t _n, const node_t *children[_n]) {
    if (func->type != NODE_ATOM) {
        return false;
    }
    if (str_equals(func->u.atom.value, STR("#do"))) {
        // todo: extract
        const node_t *bodyNode = children[1];
        assert(bodyNode->type == NODE_LIST_BEGIN);
        const size_t n = bodyNode->u.list.size;
        const node_t *bodyChildren = NODE_LIST_CHILDREN(bodyNode);
        for (size_t i = 0; i < n; ++i) {
            const bool last = i == n - 1;
            const node_t *it = node_deref(ctx->ctx, &bodyChildren[i]);
            const return_t out = last ? ret : (return_t) {
                    .tag = RETURN_TEMPORARY,
                    .u.temporary.val = node_id(ctx->ctx, it)
            };
            if (!last) {
                TAB();
                return_declare(ctx, state, out);
                OUT(ctx, "\n");
            }
            TAB();
            visit_node(ctx, state, out, it);
            if (!last) {
                OUT(ctx, "\n");
            }
        }
    } else if (str_equals(func->u.atom.value, STR("#if"))) {
        const node_t *predNode = children[1];
        const node_t *bodyNode = children[2];
        const return_t out = (return_t) {.tag = RETURN_TEMPORARY, .u.temporary.val = node_id(ctx->ctx, predNode)};
        // don't need first TAB()
        return_declare(ctx, state, out);
        OUT(ctx, "\n");

        TAB();
        visit_node(ctx, state, out, predNode);
        OUT(ctx, "\n");

        TAB();
        OUT(ctx, "if (_%lu) {\n", node_id(ctx->ctx, predNode));
        {
            state.depth++;
            // todo: same as #do
            visit_node(ctx, state, ret, bodyNode);
            OUT(ctx, "\n");
            state.depth--;
        }
        TAB();
        OUT(ctx, "}");
    } else if (str_equals(func->u.atom.value, STR("#while"))) {
        const node_t *predNode = children[1];
        const node_t *bodyNode = children[2];
        const return_t out = (return_t) {.tag = RETURN_TEMPORARY, .u.temporary.val = node_id(ctx->ctx, predNode)};
        // don't need first TAB()
        return_declare(ctx, state, out);
        OUT(ctx, "\n");

        TAB();
        visit_node(ctx, state, out, predNode);
        OUT(ctx, "\n");

        TAB();
        OUT(ctx, "while (_%lu) {\n", node_id(ctx->ctx, predNode));
        {
            state.depth++;
            // todo: same as #do
            visit_node(ctx, state, ret, bodyNode);
            OUT(ctx, "\n");

            TAB();
            visit_node(ctx, state, out, predNode);
            OUT(ctx, "\n");

            state.depth--;
        }
        TAB();
        OUT(ctx, "}");
    } else {
        return false;
    }
    return true;
}
