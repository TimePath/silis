#pragma once

#include <lib/fs.h>
#include <lib/string.h>
#include <lib/vector.h>

#include "interpreter.h"

typedef struct {
    const Interpreter *compilation;
    InterpreterFileNodeRef head;
    size_t _i, _n;
} NodeList;

bool NodeList_next(NodeList *self, InterpreterFileNodeRef *out);

InterpreterFileNodeRef NodeList_get(NodeList *self, size_t index);

NodeList NodeList_iterator(const Interpreter *compilation, InterpreterFileNodeRef list);
