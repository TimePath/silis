#pragma once

#include <lib/buffer.h>
#include <lib/string.h>
#include <lib/trie.h>

#include "phases/parse.inc.h"

typedef struct ctx_s ctx_t;

void ctx_init(ctx_t *self);

// Nodes

typedef enum {
    NODE_INVALID,
    /**
     * (
     */ NODE_LIST_BEGIN,
    /**
     * )
     */ NODE_LIST_END,
    /**
     * Generated at runtime
     * Index to address of expression in flat AST
     */ NODE_REF,
    /**
     * A word
     */ NODE_ATOM,
    /**
     * An integer
     */ NODE_INTEGRAL,
    /**
     * A string
     */ NODE_STRING,
} node_e;

typedef struct {
    size_t val;
} node_id;

typedef struct {
    node_e kind;
    uint8_t padding[4];
    union {
        /// NODE_LIST_BEGIN
        struct {
            size_t size;
            /// indices for first and last child
            /// 0 means null
            size_t begin, end;
        } list;
        /// NODE_LIST_END
        struct {
            uint8_t padding;
        } list_end;
        /// NODE_REF
        struct {
            node_id value;
        } ref;
        /// NODE_ATOM
        struct {
            String value;
        } atom;
        /// NODE_INTEGRAL
        struct {
            size_t value;
        } integral;
        /// NODE_STRING
        struct {
            String value;
        } string;
    } u;
} node_t;

#define node_list_children(ptr) ((ptr) + 1)

const node_t *node_get(const ctx_t *ctx, node_id ref);

node_id node_ref(const ctx_t *ctx, const node_t *it);

const node_t *node_deref(const ctx_t *ctx, const node_t *it);

// AST

/// begin a new list
/// \param ctx compiler context
/// \return token to current list
size_t ast_parse_push(ctx_t *ctx);

void ast_push(ctx_t *ctx, node_t it);

void ast_parse_pop(ctx_t *ctx, size_t tok);

// Types

typedef enum {
    TYPE_INVALID,
    /**
     * The leaf types of the type system
     */ TYPE_OPAQUE,
    /**
     * A function, represented as arg0 -> argN -> ret
     */ TYPE_FUNCTION,
    /**
     * Structs. TODO
     */ TYPE_AGGREGATE,
    /**
     * Pointers. TODO
     */ TYPE_POINTER,
} type_e;

typedef struct {
    size_t value;
} type_id;

typedef struct {
    type_e kind;
    uint8_t padding[4];
    union {
        /// TYPE_OPAQUE
        struct {
            size_t size;
        } opaque;
        /// TYPE_FUNCTION
        struct {
            type_id in, out;
        } func;
        /// TYPE_AGGREGATE
        struct {
            /// index to self
            type_id self;
            /// index to next child, or 0
            type_id next;
        } aggregate;
        /// TYPE_POINTER
        struct {
            type_id pointee;
        } pointer;
    } u;
} type_t;

type_id type_new(ctx_t *ctx, type_t it);

type_id type_func_new(ctx_t *ctx, type_id *argv, size_t n);

type_id type_func_ret(const ctx_t *ctx, const type_t *T);

size_t type_func_argc(const ctx_t *ctx, const type_t *T);

const type_t *type_lookup(const ctx_t *ctx, type_id id);

// Values

typedef struct value_s value_t;

typedef value_t (*intrinsic_t)(ctx_t *ctx, const value_t *value);

struct value_s {
    type_id type;
    union {
        struct {
            void *value;
        } voidp;

        struct {
            size_t value;
        } integral;

        struct {
            String value;
        } string;

        struct {
            intrinsic_t value;
        } intrinsic;

        struct {
            node_id value;
        } expr;

        struct {
            type_id value;
        } type;

        struct {
            node_id value;
            node_id arglist;
        } func;
    } u;
};

value_t value_from(const ctx_t *ctx, const node_t *n);

// Symbols

typedef struct {
    type_id type; // could be removed if values could be tagged as undefined
    value_t value;
    struct {
        /// intrinsic, can't be compiled
        bool intrinsic : 1;
        /// native declaration (libc)
        bool native : 1;
        /// interpreter variable (function call)
        bool eval : 1;
        uint8_t padding : 5;
    } flags;
    uint8_t padding[7];
} sym_t;

Trie_instantiate(sym_t);
typedef struct {
    Trie(sym_t) t;
    size_t parent;
} sym_scope_t;

Vector_instantiate(sym_scope_t);
typedef struct symbols_s {
    Vector(sym_scope_t) scopes;
} symbols_t;

void sym_push(ctx_t *ctx, size_t parent);

void sym_pop(ctx_t *ctx);

bool sym_lookup(const ctx_t *ctx, String ident, sym_t *out);

void sym_def(ctx_t *ctx, String ident, sym_t sym);

// Intrinsics

typedef void (*ctx_register_t)(ctx_t *ctx);

Vector_instantiate(ctx_register_t);
Slice_instantiate(ctx_register_t);
extern Vector(ctx_register_t) intrinsics;

void ctx_init_intrinsic(ctx_t *self, String name, type_id T, intrinsic_t func);

// State

Vector_instantiate(type_t);
Vector_instantiate(value_t);
Vector_instantiate(sym_t);
Vector_instantiate(size_t);
Vector_instantiate(node_t);
Slice_instantiate(node_t);

struct ctx_s {
    struct {
        struct {
            Vector(type_t) all;
            /// can be produced with `()`
            type_id t_unit;
            /// typedefs have this type
            type_id t_type;
            /// unevaluated code has this type
            /// when calling such a function, quasiquote the input
            /// todo: more specific types
            /// expr<void> ; returns void
            /// expr<T> ; returns T, whatever that is
            type_id t_expr;
            /// strings
            type_id t_string;
            /// integers
            type_id t_int;
            /// index of last intrinsic type
            size_t end_intrinsics;
        } types;
        symbols_t symbols;
    } state;
    struct {
        size_t list_parent_idx;
        Vector(node_t) out;
    } parse;
    struct {
        /// ordered list of expressions to be evaluated at runtime
        /// starting from 1
        Vector(node_t) out;
    } flatten;
    struct {
        Vector(value_t) stack;
    } eval;
};
