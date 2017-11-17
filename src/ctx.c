#include "ctx.h"
#include "phases/parse.h"
#include "phases/parse.inc.h"

#include <assert.h>
#include <alloca.h>
#include <malloc.h>

static sym_trie_t *sym_trie_new();

void ctx_init(ctx_t *self) {
    self->state.symbols = sym_trie_new();
    self->state.types.t_unit = type_new(self, (type_t) {
            .type = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    assert(self->state.types.t_unit.value == 0);
    self->state.types.t_expr = type_new(self, (type_t) {
            .type = TYPE_OPAQUE,
            .u.opaque.size = 0,
    });
    self->state.types.t_type = type_new(self, (type_t) {
            .type = TYPE_OPAQUE,
            .u.opaque.size = sizeof(type_id),
    });
    self->state.types.t_string = type_new(self, (type_t) {
            .type = TYPE_OPAQUE,
            .u.opaque.size = sizeof(const char *),
    });
    sym_def(self, STR("#types/string"), (sym_t) {
            .type = self->state.types.t_type,
            .value.type = self->state.types.t_type,
            .value.u.integral.value = self->state.types.t_string.value,
    });

    self->state.types.t_int = type_new(self, (type_t) {
            .type = TYPE_OPAQUE,
            .u.opaque.size = sizeof(int),
    });
    sym_def(self, STR("#types/int"), (sym_t) {
            .type = self->state.types.t_type,
            .value.type = self->state.types.t_type,
            .value.u.integral.value = self->state.types.t_int.value,
    });

    vec_loop(intrinsics, i, 0) {
        intrinsics.data[i](self);
    }
    self->state.types.end_intrinsics = self->state.types.all.size - 1;
}

// AST

size_t ast_parse_push(ctx_t *self) {
    const size_t thisIdx = self->parse.out.size;
    const size_t parentIdx = self->parse.list_parent_idx;
    self->parse.list_parent_idx = thisIdx;
    if (parentIdx) {
        node_t *parent = &self->parse.out.data[parentIdx];
        parent->u.list.end = thisIdx;
        parent->u.list.size += 1;
    }
    node_t it = (node_t) {
            .type = NODE_LIST_BEGIN,
    };
    vec_push(&self->parse.out, it);
    return parentIdx;
}

void ast_push(ctx_t *self, node_t it) {
    const size_t thisIdx = self->parse.out.size;
    const size_t parentIdx = self->parse.list_parent_idx;
    node_t *parent = &self->parse.out.data[parentIdx];
    if (!parent->u.list.begin) {
        parent->u.list.begin = thisIdx;
    }
    parent->u.list.end = thisIdx;
    parent->u.list.size += 1;
    vec_push(&self->parse.out, it);
}

void ast_parse_pop(ctx_t *self, size_t tok) {
    self->parse.list_parent_idx = tok;
    node_t it = (node_t) {
            .type = NODE_LIST_END,
    };
    vec_push(&self->parse.out, it);
}

// Types

type_id type_new(ctx_t *ctx, type_t it) {
    vec_push(&ctx->state.types.all, it);
    return (type_id) {ctx->state.types.all.size - 1};
}

type_id type_func_new(ctx_t *ctx, type_id *argv, size_t n) {
    size_t i = n - 1;
    type_id ret = argv[i];
    while (i-- > 0) {
        type_id in = argv[i];
        ret = type_new(ctx, (type_t) {
                .type = TYPE_FUNCTION,
                .u.func.in = in,
                .u.func.out = ret,
        });
    }
    return ret;
}

size_t type_func_argc(const ctx_t *ctx, type_id id) {
    size_t argc = 0;
    for (type_t it; (it = ctx->state.types.all.data[id.value]).type == TYPE_FUNCTION; ++argc) {
        id = it.u.func.out;
    };
    return argc;
}

// Values

value_t val_from(const ctx_t *ctx, const node_t *n) {
    switch (n->type) {
        default:
            assert(false);
            return (value_t) {0};
        case NODE_ATOM: {
            const sym_t *maybe = sym_lookup(ctx, n->u.atom.value);
            assert(maybe);
            return maybe->value;
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
    }
}

// Symbols

/// map of parser characters to 0 or char id
static uint8_t sym_trie_chars[256];

STATIC_INIT {
    uint8_t n = 0;
    for (size_t i = 0; i < 256; ++i) {
        sym_trie_chars[i] = parse_chars[i] ? ++n : (uint8_t) 0;
    }
}

typedef struct {
    sym_t value;
    uint16_t children[PARSE_NPOT]; // consider alphabet reduction
} sym_trie_node_t;

instantiate_vec_t(sym_trie_node_t);

struct sym_trie_s {
    vec_t(sym_trie_node_t) trie;
};

static sym_trie_t *sym_trie_new() {
    sym_trie_t *self = malloc(sizeof(sym_trie_t));
    *self = (sym_trie_t) {0};
    const sym_trie_node_t root = (sym_trie_node_t) {0};
    vec_push(&self->trie, root);
    return self;
}

sym_t *sym_trie_at(ctx_t *ctx, string_view_t ident) {
    vec_t(sym_trie_node_t) *vec = &ctx->state.symbols->trie;
    sym_trie_node_t *n = &vec->data[0];
    str_loop(ident, it, 0) {
        uint8_t i = sym_trie_chars[(int) *it];
        assert(i);
        i -= 1;
        const uint16_t idx = n->children[i];
        if (idx != 0) {
            n = &vec->data[idx];
            continue;
        }
        const size_t end_ = vec->size;
        const int uint16_max = (uint16_t) -1;
        assert(end_ < uint16_max);
        const uint16_t end = (uint16_t) end_;
        const sym_trie_node_t empty = (sym_trie_node_t) {0};
        n->children[i] = end;
        vec_push(vec, empty);
        n = &vec->data[end];
    }
    return &n->value;
}

const sym_t *sym_lookup(const ctx_t *ctx, string_view_t ident) {
    sym_t *ret = sym_trie_at((ctx_t *) ctx, ident);
    if (!ret->value.type.value) {
        return NULL;
    }
    return ret;
}

void sym_def(ctx_t *ctx, string_view_t ident, sym_t sym) {
    sym_t *ret = sym_trie_at(ctx, ident);
    *ret = (sym_t) {
            .type = sym.type,
            .value = sym.value,
    };
}

// Intrinsics

vec_t(ctx_register_t) intrinsics = {0};

void ctx_init_intrinsic(ctx_t *self, string_view_t name, type_id T, intrinsic_t func) {
    sym_def(self, name, (sym_t) {
            .type = T,
            .value.type = T,
            .value.u.intrinsic.value = func,
    });
}
