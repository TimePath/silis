#include "ctx.h"

#include <assert.h>
#include <alloca.h>

void ctx_init(ctx_t *self) {
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

const sym_t *sym_lookup(const ctx_t *ctx, string_view_t ident) {
    vec_loop(ctx->state.symbols, i, 0) {
        const sym_t *it = &ctx->state.symbols.data[i];
        if (str_equals(it->name, ident)) {
            return it;
        }
    }
    return NULL;
}

void sym_def(ctx_t *ctx, string_view_t ident, sym_t sym) {
    sym_t *ret = NULL;
    vec_loop(ctx->state.symbols, i, 0) {
        sym_t *it = &ctx->state.symbols.data[i];
        if (str_equals(it->name, ident)) {
            ret = it;
            break;
        }
    }
    if (!ret) {
        sym_t dummy = (sym_t) {0};
        vec_push(&ctx->state.symbols, dummy);
        ret = &ctx->state.symbols.data[ctx->state.symbols.size - 1];
    }
    *ret = (sym_t) {
            .name = ident,
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
