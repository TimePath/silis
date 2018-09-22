#pragma once

#include <compiler/env.h>
#include <compiler/symbols.h>
#include "../../types.h"
#include "../../node.h"
#include "../../value.h"

typedef struct {
    Env env;
} eval_input;

typedef void eval_output;

eval_output do_eval(eval_input in);

value_t eval_node(Env env, const node_t *it);

value_t eval_list_block(Env env, const node_t *it);
