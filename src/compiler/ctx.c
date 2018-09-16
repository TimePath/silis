#include <system.h>
#include "ctx.h"

#include <lib/string.h>
#include <lib/slice.h>

#include "phases/parse.h"

#include <compiler/intrinsics/debug/puti.h>
#include <compiler/intrinsics/debug/puts.h>
#include <compiler/intrinsics/types/func.h>
#include <compiler/intrinsics/cond.h>
#include <compiler/intrinsics/define.h>
#include <compiler/intrinsics/do.h>
#include <compiler/intrinsics/extern.h>
#include <compiler/intrinsics/func.h>
#include <compiler/intrinsics/minus.h>
#include <compiler/intrinsics/plus.h>

void ctx_init(ctx_t *self) {
    sym_push(self, 0);
    self->state.types.t_unit = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    assert(self->state.types.t_unit.value == 0 && "unit has type id 0");
    self->state.types.t_expr = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    self->state.types.t_type = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(type_id),
    });
    self->state.types.t_string = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(const char *),
    });
    sym_def(self, STR("#types/string"), (sym_t) {
            .type = self->state.types.t_type,
            .value.type = self->state.types.t_type,
            .value.u.type.value = self->state.types.t_string,
            .flags.intrinsic = true,
    });

    self->state.types.t_int = type_new(self, (type_t) {
            .kind = TYPE_OPAQUE,
            .u.opaque.size = sizeof(size_t),
    });
    sym_def(self, STR("#types/int"), (sym_t) {
            .type = self->state.types.t_type,
            .value.type = self->state.types.t_type,
            .value.u.type.value = self->state.types.t_int,
            .flags.intrinsic = true,
    });

    intrin_debug_puti.load(self);
    intrin_debug_puts.load(self);

    intrin_types_func.load(self);

    intrin_cond.load(self);
    intrin_define.load(self);
    intrin_do.load(self);
    intrin_extern.load(self);
    intrin_func.load(self);
    intrin_minus.load(self);
    intrin_plus.load(self);

    self->state.types.end_intrinsics = Vector_size(&self->state.types.all) - 1;
}

// Nodes

const node_t *node_get(const ctx_t *ctx, node_id ref) {
    return &Vector_data(&ctx->flatten.out)[ref.val];
}

node_id node_ref(const ctx_t *ctx, const node_t *it) {
    assert(it >= Vector_data(&ctx->flatten.out) && it <= &Vector_data(&ctx->flatten.out)[Vector_size(&ctx->flatten.out) - 1]);
    return (node_id) {(size_t) (it - Vector_data(&ctx->flatten.out))};
}

const node_t *node_deref(const ctx_t *ctx, const node_t *it) {
    if (it->kind == NODE_REF) {
        const node_t *ret = node_get(ctx, it->u.ref.value);
        assert(ret->kind != NODE_REF && "no double refs");
        return ret;
    }
    return it;
}

// AST

size_t ast_parse_push(ctx_t *ctx) {
    Vector(node_t) *out = &ctx->parse.out;
    const size_t thisIdx = Vector_size(out);
    const size_t parentIdx = ctx->parse.list_parent_idx;
    ctx->parse.list_parent_idx = thisIdx;
    if (parentIdx) {
        node_t *parent = &Vector_data(out)[parentIdx];
        parent->u.list.end = thisIdx;
        parent->u.list.size += 1;
    }
    node_t it = (node_t) {
            .kind = NODE_LIST_BEGIN,
    };
    Vector_push(out, it);
    return parentIdx;
}

void ast_push(ctx_t *ctx, node_t it) {
    Vector(node_t) *out = &ctx->parse.out;
    const size_t thisIdx = Vector_size(out);
    const size_t parentIdx = ctx->parse.list_parent_idx;
    node_t *parent = &Vector_data(out)[parentIdx];
    if (!parent->u.list.begin) {
        parent->u.list.begin = thisIdx;
    }
    parent->u.list.end = thisIdx;
    parent->u.list.size += 1;
    Vector_push(out, it);
}

void ast_parse_pop(ctx_t *ctx, size_t tok) {
    Vector(node_t) *out = &ctx->parse.out;
    ctx->parse.list_parent_idx = tok;
    node_t it = (node_t) {
            .kind = NODE_LIST_END,
    };
    Vector_push(out, it);
}

// Types

type_id type_new(ctx_t *ctx, type_t it) {
    Vector_push(&ctx->state.types.all, it);
    return (type_id) {Vector_size(&ctx->state.types.all) - 1};
}

type_id type_func_new(ctx_t *ctx, type_id *argv, size_t n) {
    size_t i = n - 1;
    type_id ret = argv[i];
    while (i-- > 0) {
        type_id in = argv[i];
        ret = type_new(ctx, (type_t) {
                .kind = TYPE_FUNCTION,
                .u.func.in = in,
                .u.func.out = ret,
        });
    }
    return ret;
}

type_id type_func_ret(const ctx_t *ctx, const type_t *T) {
    type_id ret = {.value = 0};
    for (const type_t *it = T; it->kind == TYPE_FUNCTION; it = type_lookup(ctx, it->u.func.out)) {
        ret = it->u.func.out;
    }
    return ret;
}

size_t type_func_argc(const ctx_t *ctx, const type_t *T) {
    size_t argc = 0;
    for (const type_t *it = T; it->kind == TYPE_FUNCTION; it = type_lookup(ctx, it->u.func.out)) {
        ++argc;
    }
    return argc;
}

const type_t *type_lookup(const ctx_t *ctx, type_id id) {
    return &Vector_data(&ctx->state.types.all)[id.value];
}

// Values

value_t value_from(const ctx_t *ctx, const node_t *n) {
    switch (n->kind) {
        case NODE_ATOM: {
            const String ident = n->u.atom.value;
            sym_t symbol;
            bool status = sym_lookup(ctx, ident, &symbol);
            (void) status;
            assert(status && "symbol is defined");
            return symbol.value;
        }
        case NODE_INTEGRAL:
            return (value_t) {
                    .type = ctx->state.types.t_int,
                    .u.integral.value = n->u.integral.value,
            };
        case NODE_STRING:
            return (value_t) {
                    .type = ctx->state.types.t_string,
                    .u.string.value = n->u.string.value,
            };
        case NODE_INVALID:
        case NODE_LIST_BEGIN:
        case NODE_LIST_END:
        case NODE_REF:
            break;
    }
    assert(false);
    return (value_t) {0};
}

// Symbols

static sym_scope_t sym_trie_new(size_t parent) {
    sym_scope_t self = (sym_scope_t) { .parent = parent };
    Trie_new(sym_t, &self.t);
    return self;
}

static void sym_trie_delete(sym_scope_t *self) {
    Vector_delete(&self->t.nodes);
    Vector_delete(&self->t.entries);
}

static bool sym_trie_get(sym_scope_t *self, String ident, sym_t *out) {
    return Trie_get((void *) &self->t, ident.bytes, out);
}

static void sym_trie_set(sym_scope_t *self, String ident, sym_t val) {
    Trie_set((void *) &self->t, ident.bytes, &val, sizeof(TrieNode(sym_t)));
}

void sym_push(ctx_t *ctx, size_t parent) {
    symbols_t *symbols = &ctx->state.symbols;
    sym_scope_t newscope = sym_trie_new(parent);
    Vector_push(&symbols->scopes, newscope);
}

void sym_pop(ctx_t *ctx) {
    symbols_t *symbols = &ctx->state.symbols;
    sym_trie_delete(&Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1]);
    Vector_pop(&symbols->scopes);
}

bool sym_lookup(const ctx_t *ctx, String ident, sym_t *out) {
    const symbols_t *symbols = &ctx->state.symbols;
    size_t i = Vector_size(&symbols->scopes) - 1;
    for (;;) {
        sym_scope_t *it = &Vector_data(&symbols->scopes)[i];
        if (sym_trie_get(it, ident, out)) {
            return true;
        }
        const size_t next = it->parent;
        if (i == next) {
            return false;
        }
        i = next;
    }
}

void sym_def(ctx_t *ctx, String ident, sym_t sym) {
    symbols_t *symbols = &ctx->state.symbols;
    sym_scope_t *top = &Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1];
    sym_trie_set(top, ident, sym);
}

// Intrinsics

void ctx_init_intrinsic(ctx_t *self, String name, type_id T, intrinsic_t func) {
    sym_def(self, name, (sym_t) {
            .type = T,
            .value.type = T,
            .value.u.intrinsic.value = func,
            .flags.intrinsic = true,
    });
}
