#pragma once

#include "lib/string.h"
#include "lib/buffer.h"

#include <stdint.h>

// AST

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
    node_e type;
    string_view_t text;
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
            bool _: 1;
        } list_end;
        /// NODE_REF
        struct {
            size_t value;
        } ref;
        /// NODE_ATOM
        struct {
            string_view_t value;
        } atom;
        /// NODE_INTEGRAL
        struct {
            size_t value;
        } integral;
        /// NODE_STRING
        struct {
            string_view_t value;
        } string;
    } u;
} node_t;

// Types

typedef struct {
    size_t value;
} type_id;

typedef enum {
    TYPE_INVALID,
    /**
     * The leaf types of the type system
     */ TYPE_OPAQUE,
    /**
     * A function, represented as arg0 -> arg1 -> argN
     */ TYPE_FUNCTION,
    /**
     * Structs. TODO
     */ TYPE_AGGREGATE,
    /**
     * Pointers. TODO
     */ TYPE_POINTER,
} type_e;

typedef struct {
    type_e type;
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

// Values

struct ctx_s;

typedef struct value_s (*intrinsic_t)(struct ctx_s *, const struct value_s *);

typedef struct value_s {
    type_id type;
    union {
        struct {
            void *value;
        } voidp;

        struct {
            size_t value;
        } integral;

        struct {
            string_view_t value;
        } string;

        struct {
            intrinsic_t value;
        } intrinsic;

        struct {
            size_t value;
            size_t arglist;
        } func;
    } u;
} value_t;

// Symbols

typedef struct {
    string_view_t name;
    type_id type;
    value_t value;
} sym_t;

// State

instantiate_vec_t(type_t);
instantiate_vec_t(value_t);
instantiate_vec_t(sym_t);
instantiate_vec_t(size_t);
instantiate_vec_t(node_t);

typedef struct ctx_s {
    struct {
        buffer_t buffer;
    } source;
    struct {
        struct {
            vec_t(type_t) all;
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
        vec_t(sym_t) symbols;
        vec_t(node_t) functions;
    } state;
    struct {
        size_t list_parent_idx;
        vec_t(node_t) out;
    } parse;
    struct {
        /// ordered list of expressions to be evaluated at runtime
        /// starting from 1
        vec_t(node_t) out;
    } flatten;
    struct {
        vec_t(value_t) stack;
    } eval;
} ctx_t;

void ctx_init(ctx_t *self);

// AST

/// begin a new list
/// \param self
/// \return token to current list
size_t ast_parse_push(ctx_t *self);

void ast_push(ctx_t *self, node_t it);

void ast_parse_pop(ctx_t *self, size_t tok);

// Types

type_id type_new(ctx_t *ctx, type_t it);

type_id type_func_new(ctx_t *ctx, type_id *argv, size_t n);

size_t type_func_argc(const ctx_t *ctx, type_id id);

// Values

value_t val_from(const ctx_t *ctx, const node_t *n);

// Symbols

const sym_t *sym_lookup(const ctx_t *ctx, string_view_t ident);

// Intrinsics

typedef void (*ctx_register_t)(ctx_t *ctx);

instantiate_vec_t(ctx_register_t);
extern vec_t(ctx_register_t) intrinsics;

void ctx_init_intrinsic(ctx_t *self, string_view_t name, type_id T, intrinsic_t func);
