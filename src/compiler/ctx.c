#include <system.h>
#include "ctx.h"

#include <lib/string.h>
#include <lib/slice.h>

#include "phases/parse.h"

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
    Slice_loop(&Vector_toSlice(ctx_register_t, &intrinsics), i) {
        Vector_data(&intrinsics)[i](self);
    }
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
            const sym_t *symbol = sym_lookup(ctx, ident);
            assert(symbol && "symbol is defined");
            return symbol->value;
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

/// map of parser characters to 0 or char id
static uint8_t sym_trie_chars[256];

STATIC_INIT(sym_trie_chars) {
    uint8_t n = 0;
    for (size_t i = 0; i < 256; ++i) {
        sym_trie_chars[i] = parse_char(i) ? ++n : (uint8_t) 0;
    }
}

static sym_trie_t sym_trie_new(void) {
    sym_trie_t self = (sym_trie_t) {0};
    const sym_trie_node_t root = (sym_trie_node_t) {0};
    Vector_push(&self.nodes, root);
    return self;
}

enum {
    TRIE_NONE = 0,
    TRIE_CREATE_INTERMEDIATES = 1,
};

static sym_trie_node_t *sym_trie_at(sym_trie_t *self, String ident, uint8_t flags) {
    const StringEncoding *enc = ident.encoding;
    sym_trie_node_t *n = &Vector_data(&self->nodes)[0];
    for (Slice(uint8_t) it = ident.bytes; Slice_begin(&it) != Slice_end(&it); it = enc->next(it)) {
        const size_t c = enc->get(it);
        assert(c < ARRAY_LEN(sym_trie_chars));
        uint8_t i = sym_trie_chars[c];
        assert(i && "char is defined");
        i -= 1;
        const uint16_t idx = n->children[i];
        if (idx != 0) {
            n = &Vector_data(&self->nodes)[idx];
            continue;
        }
        if (!(flags & TRIE_CREATE_INTERMEDIATES)) {
            return NULL;
        }
        const size_t end_ = Vector_size(&self->nodes);
        const size_t uint16_max = (uint16_t) -1;
        (void) uint16_max;
        assert(end_ < uint16_max && "size is constrained");
        const uint16_t end = (uint16_t) end_;
        const sym_trie_node_t empty = (sym_trie_node_t) {0};
        n->children[i] = end;
        Vector_push(&self->nodes, empty);
        n = &Vector_data(&self->nodes)[end];
    }
    if (!(flags & TRIE_CREATE_INTERMEDIATES) && !n->value.type.value) {
        return NULL;
    }
    return n;
}

void sym_push(ctx_t *ctx, size_t parent) {
    sym_trie_t newscope = sym_trie_new();
    newscope.parent = parent;
    symbols_t *symbols = &ctx->state.symbols;
    Vector_push(&symbols->scopes, newscope);
}

void sym_pop(ctx_t *ctx) {
    symbols_t *symbols = &ctx->state.symbols;
    Vector_delete(&Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1].nodes);
    Vector_pop(&symbols->scopes);
}

const sym_t *sym_lookup(const ctx_t *ctx, String ident) {
    const symbols_t *symbols = &ctx->state.symbols;
    size_t i = Vector_size(&symbols->scopes) - 1;
    for (;;) {
        sym_trie_t *self = &Vector_data(&symbols->scopes)[i];
        sym_trie_node_t *ret = sym_trie_at(self, ident, TRIE_NONE);
        if (ret) {
            return &ret->value;
        }
        const size_t next = self->parent;
        if (i == next) {
            return NULL;
        }
        i = next;
    }
}

void sym_def(ctx_t *ctx, String ident, sym_t sym) {
    symbols_t *symbols = &ctx->state.symbols;
    sym_trie_t *self = &Vector_data(&symbols->scopes)[Vector_size(&symbols->scopes) - 1];
    sym_trie_node_t *ret = sym_trie_at(self, ident, TRIE_CREATE_INTERMEDIATES);
    ret->value = sym;
    sym_trie_entry_t e = (sym_trie_entry_t) {
            .key = ident,
            .value = (uint16_t) (ret - Vector_data(&self->nodes)),
    };
    Vector_push(&self->list, e);
}

// Intrinsics

Vector(ctx_register_t) intrinsics = {0};

void ctx_init_intrinsic(ctx_t *self, String name, type_id T, intrinsic_t func) {
    sym_def(self, name, (sym_t) {
            .type = T,
            .value.type = T,
            .value.u.intrinsic.value = func,
            .flags.intrinsic = true,
    });
}
