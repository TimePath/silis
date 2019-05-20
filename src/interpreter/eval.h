#pragma once

#include "interpreter.h"
#include "nodelist.h"
#include "symbols.h"
#include "types.h"
#include "value.h"

typedef struct {
    Interpreter *interpreter;
    InterpreterFileNodeRef entry;
} eval_input;

typedef void eval_output;

eval_output do_eval(eval_input in);

Value eval_node(Interpreter *interpreter, InterpreterFileNodeRef it);

Value eval_list_block(Interpreter *interpreter, InterpreterFileNodeRef it);

Value func_call(Interpreter *interpreter, Value func, Slice(Value) argv, InterpreterFileNodeRef it);
