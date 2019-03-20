#pragma once

#include <lib/vector.h>

#include "env.h"
#include "node.h"
#include "type.h"

typedef struct {
    type_id type;
    compilation_node_ref node;
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
            struct Intrinsic_s *value;
        } intrinsic;

        struct {
            compilation_node_ref value;
        } expr;

        struct {
            type_id value;
        } type;

        struct {
            compilation_node_ref value;
            compilation_node_ref arglist;
        } func;
    } u;
    struct {
        /// unknown value
        bool abstract : 1;
        /// intrinsic, can't be compiled as-is
        bool intrinsic : 1;
        /// native declaration (libc function, or other external symbol)
        bool native : 1;
        uint8_t _padding : 5;
    } flags;
    uint8_t _padding[7];
} value_t;

Slice_instantiate(value_t);
Vector_instantiate(value_t);

value_t value_from(Env env, compilation_node_ref n);
