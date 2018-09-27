#pragma once

#include <lib/vector.h>

#include "env.h"
#include "node.h"
#include "type.h"

typedef struct {
    type_id type;
    const node_t *node;
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
    struct {
        /// unknown value
        bool abstract : 1;
        /// intrinsic, can't be compiled as-is
        bool intrinsic : 1;
        /// native declaration (libc function, or other external symbol)
        bool native : 1;
        uint8_t padding : 5;
    } flags;
    uint8_t padding[7];
} value_t;

Vector_instantiate(value_t);
Slice_instantiate(value_t);

value_t value_from(Env env, const node_t *n);
