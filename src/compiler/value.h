#pragma once

#include <lib/vector.h>

#include "env.h"
#include "node.h"
#include "type.h"

typedef struct value_s value_t;

typedef value_t (*intrinsic_t)(Env env, const value_t *value);

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

Vector_instantiate(value_t);

value_t value_from(Env env, const node_t *n);
