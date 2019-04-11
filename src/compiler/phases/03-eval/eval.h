#pragma once

#include <compiler/env.h>
#include <compiler/node.h>
#include <compiler/symbols.h>
#include <compiler/types.h>
#include <compiler/value.h>

typedef struct {
    Env env;
    compilation_node_ref entry;
} eval_input;

typedef void eval_output;

eval_output do_eval(eval_input in);

value_t eval_node(Env env, compilation_node_ref it);

value_t eval_list_block(Env env, compilation_node_ref it);
