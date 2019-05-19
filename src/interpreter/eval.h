#pragma once

#include "env.h"
#include "node.h"
#include "symbols.h"
#include "types.h"
#include "value.h"

typedef struct {
    Env env;
    compilation_node_ref entry;
} eval_input;

typedef void eval_output;

eval_output do_eval(eval_input in);

value_t eval_node(Env env, compilation_node_ref it);

value_t eval_list_block(Env env, compilation_node_ref it);

value_t func_call(Env env, value_t func, Slice(value_t) argv, compilation_node_ref it);