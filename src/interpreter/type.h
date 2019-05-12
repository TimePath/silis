#pragma once

#include <lib/vector.h>

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
    PADDING(4)
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

#define type_t_delete(self) ((void) (self))

Slice_instantiate(type_t);
Vector_instantiate(type_t);
